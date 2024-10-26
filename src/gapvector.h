#ifndef GAPVECTOR_H
#define GAPVECTOR_H

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "logger.h"

// Gap buffer data structure implementation: https://en.wikipedia.org/wiki/Gap_buffer
// TODO: Implement moving left/right

template <typename T>
concept Fundamental = std::is_fundamental_v<T>;

template <Fundamental T = char, class Allocator = std::allocator<T>>
class Gapvector {
   public:
    // Member types
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

   private:
    // Iterator built from: https://medium.com/p/fc5b994462c6#90c4
    template <typename PointerType>
    class IteratorTemplate {
       public:
        static const bool is_const = std::is_const_v<std::remove_pointer_t<PointerType>>;
        using iterator_category = std::contiguous_iterator_tag;
        using gapvector_pointer = std::conditional<is_const, const Gapvector*, Gapvector*>::type;

        using value_type =
            std::conditional<is_const, const Gapvector::value_type, Gapvector::value_type>::type;
        using element_type = std::conditional<is_const, const value_type, value_type>::type;

        using pointer = PointerType;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;

        gapvector_pointer gv;
        PointerType ptr;

        explicit IteratorTemplate() = default;
        explicit IteratorTemplate(gapvector_pointer self, PointerType input_ptr)
            : gv(self), ptr(input_ptr) {}

        IteratorTemplate& operator++() {
            ptr += 1;
            if (ptr == gv->gapStart) {
                ptr = gv->gapEnd;
            }
            return *this;
        }

        // pre-increment
        IteratorTemplate operator++(int) {
            IteratorTemplate tmp = *this;
            ++(*this);
            if (ptr == gv->gapStart) {
                ptr = gv->gapEnd;
            }
            return tmp;
        }

        IteratorTemplate& operator--() {
            ptr -= 1;
            if (ptr == gv->gapEnd) {
                ptr = gv->gapStart - 1;
            }
            return *this;
        }

        // pre-increment
        IteratorTemplate operator--(int) {
            IteratorTemplate tmp = *this;
            --(*this);
            if (ptr == gv->gapEnd) {
                ptr = gv->gapStart - 1;
            }
            return tmp;
        }

        IteratorTemplate operator+(const int val) {
            IteratorTemplate tmp = *this;
            return tmp += val;
        }

        IteratorTemplate operator+(const difference_type other) const {
            return IteratorTemplate(gv, ptr + other);
        }

        friend IteratorTemplate operator+(
            const difference_type value,
            const IteratorTemplate& other) {
            return other + value;
        }

        IteratorTemplate operator-(const int val) {
            IteratorTemplate tmp = *this;
            return tmp -= val;
        }

        difference_type operator-(const IteratorTemplate& other) const {
            // return ptr - other.ptr;
            if (ptr < gv->gapStart && other.ptr < gv->gapStart) {
                return ptr - other.ptr;
            } else if (ptr > gv->gapEnd && other.ptr > gv->gapEnd) {
                return ptr - other.ptr;
            } else {
                return (gv->gapStart - other.ptr) + (ptr - gv->gapEnd);
            }
        }

        IteratorTemplate operator-(const difference_type other) const {
            return IteratorTemplate(gv, ptr - other);
        }

        friend IteratorTemplate operator-(
            const difference_type value,
            const IteratorTemplate& other) {
            return other - value;
        }

        IteratorTemplate& operator+=(difference_type n) {
            ptr += n;
            return *this;
        }

        IteratorTemplate& operator-=(difference_type n) {
            ptr -= n;
            return *this;
        }

        auto operator<=>(const IteratorTemplate&) const = default;

        reference operator*() { return *ptr; }
        PointerType operator->() const { return &operator*(); }
    };

    // Iterator member types
    using iterator = IteratorTemplate<pointer>;
    using const_iterator = IteratorTemplate<const_pointer>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   public:
    // Constructors
    constexpr explicit Gapvector() {
        bufferStart = allocator_type().allocate(32);
        bufferEnd = std::uninitialized_value_construct_n(bufferStart, 32);
        gapStart = bufferStart;
        gapEnd = bufferEnd;
    }

    constexpr explicit Gapvector(const size_type length) {
        bufferStart = allocator_type().allocate(length);
        bufferEnd = std::uninitialized_value_construct_n(bufferStart, length);
        gapStart = bufferStart;
        gapEnd = bufferEnd;
    }

    constexpr explicit Gapvector(std::string_view str) {
        bufferStart = allocator_type().allocate(str.size() + 8);
        std::uninitialized_copy_n(str.begin(), str.size(), bufferStart);

        gapStart = bufferStart + str.size();
        gapEnd = gapStart + 8;
        bufferEnd = gapEnd;
    }

    template <typename InputIt>
    constexpr Gapvector(InputIt begin, InputIt end) {
        const unsigned int len = std::distance(begin, end);

        bufferStart = allocator_type().allocate(len + 8);
        std::uninitialized_copy_n(begin, len, bufferStart);

        gapStart = bufferStart + len;
        gapEnd = gapStart + 8;
        bufferEnd = gapEnd;
    }

    // Copy Constructor
    constexpr Gapvector(const Gapvector& other) {
        bufferStart = allocator_type().allocate(other.capacity());
        std::uninitialized_copy_n(other.bufferStart, other.capacity(), bufferStart);

        gapStart = bufferStart + (other.gapStart - other.bufferStart);
        gapEnd = bufferStart + (other.gapEnd - other.bufferStart);
        bufferEnd = bufferStart + other.capacity();
    }

    // Copy Assignment
    constexpr Gapvector& operator=(const Gapvector& other) {
        if (this != &other) {
            pointer new_bufferStart = allocator_type().allocate(other.capacity());

            std::uninitialized_copy_n(other.bufferStart, other.capacity(), new_bufferStart);

            allocator_type().deallocate(bufferStart, capacity());

            bufferStart = new_bufferStart;
            gapStart = bufferStart + (other.gapStart - other.bufferStart);
            gapEnd = bufferStart + (other.gapEnd - other.bufferStart);
            bufferEnd = bufferStart + other.capacity();
        }
        return *this;
    }

    // Move Constructor
    constexpr Gapvector(Gapvector&& other)
        : bufferStart(other.bufferStart),
          gapStart(other.gapStart),
          gapEnd(other.gapEnd),
          bufferEnd(other.bufferEnd) {
        other.bufferStart = nullptr;
        other.gapStart = nullptr;
        other.gapEnd = nullptr;
        other.bufferEnd = nullptr;
    }

    // Move Assignment Operator
    constexpr Gapvector& operator=(Gapvector&& other) {
        std::destroy_n(bufferStart, capacity());
        allocator_type().deallocate(bufferStart, capacity());

        bufferStart = std::exchange(other.bufferStart, {});
        gapStart = std::exchange(other.gapStart, {});
        gapEnd = std::exchange(other.gapEnd, {});
        bufferEnd = std::exchange(other.bufferEnd, {});
        return *this;
    }

    ~Gapvector() {
        std::destroy_n(bufferStart, capacity());
        allocator_type().deallocate(bufferStart, capacity());
    }

    // Operator overloads
    friend std::ostream& operator<<(std::ostream& os, const Gapvector& gv) {
        os << "[";
        for (auto p = gv.bufferStart; p < gv.bufferEnd; p++) {
            if (p >= gv.gapStart && p <= gv.gapEnd) {
                os << " ";
            } else {
                os << *p;
            }
        }
        os << "]";

        return os;
    }

    [[nodiscard]] constexpr reference operator[](const size_type& loc) noexcept {
        if (loc < static_cast<size_type>(gapStart - bufferStart)) {
            return *(bufferStart + loc);
        } else {
            return *(gapEnd + (loc - (gapStart - bufferStart)));
        }
    }

    [[nodiscard]] constexpr const_reference operator[](const size_type& loc) const noexcept {
        if (loc < static_cast<size_type>(gapStart - bufferStart)) {
            return *(bufferStart + loc);
        } else {
            return *(gapEnd + (loc - (gapStart - bufferStart)));
        }
    }

    // Element Access
    [[nodiscard]] constexpr reference at(size_type loc) {
        if (loc >= size()) {
            throw std::out_of_range("index out of range");
        }

        // Determine the actual memory address to access
        if (loc < static_cast<size_type>(gapStart - bufferStart)) {
            // Before the gap
            return *(bufferStart + loc);
        } else {
            // After the gap
            return *(gapEnd + (loc - (gapStart - bufferStart)));
        }
    }

    [[nodiscard]] constexpr const_reference at(size_type loc) const {
        if (loc >= size()) {
            throw std::out_of_range("index out of range");
        }

        // Determine the actual memory address to access
        if (loc < static_cast<size_type>(gapStart - bufferStart)) {
            // Before the gap
            return *(bufferStart + loc);
        } else {
            // After the gap
            return *(gapEnd + (loc - (gapStart - bufferStart)));
        }
    }

    [[nodiscard]] constexpr reference front() {
        if (bufferStart == nullptr || gapStart == nullptr) {
            throw std::runtime_error("Accessing front element in an empty Gapvector");
        }
        if (gapStart == bufferStart) {
            throw std::out_of_range("Accessing front element in an empty Gapvector");
        }
        return *bufferStart;
    }

    [[nodiscard]] constexpr const_reference front() const {
        if (bufferStart == nullptr || gapStart == nullptr) {
            throw std::runtime_error("Accessing front element in an empty Gapvector");
        }
        if (gapStart == bufferStart) {
            throw std::out_of_range("Accessing front element in an empty Gapvector");
        }
        return *bufferStart;
    }

    [[nodiscard]] constexpr reference back() {
        if (bufferStart == nullptr || gapStart == nullptr) {
            throw std::runtime_error("Accessing back element in an empty Gapvector");
        }
        if (gapStart == bufferEnd) {
            throw std::out_of_range("Accessing back element in an empty Gapvector");
        }
        return *(gapStart - 1);
    }

    [[nodiscard]] constexpr const_reference back() const {
        if (bufferStart == nullptr || gapStart == nullptr) {
            throw std::runtime_error("Accessing back element in an empty Gapvector");
        }
        if (gapStart == bufferEnd) {
            throw std::out_of_range("Accessing back element in an empty Gapvector");
        }
        return *(gapStart - 1);
    }

    [[nodiscard]] const std::string to_str() const noexcept {
        std::string ret;
        ret.reserve(size());
        ret.append(bufferStart, (gapStart - bufferStart));
        ret.append(gapEnd, (bufferEnd - gapEnd));
        return ret;
    }

    [[nodiscard]] std::string line(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("index out of range");
        }
        if (empty()) {
            throw std::runtime_error("Cannot pull line from empty gapvector");
        }

        // decrement pos to get it to the correct location when 0-based
        pos--;
        auto start_rit = std::find(std::make_reverse_iterator(begin() + pos), rend(), '\n');
        auto start_it = (start_rit == rend()) ? begin() : start_rit.base();
        auto end_it = std::find(begin() + pos, end(), '\r');

        return std::string(start_it, end_it);
    }

    [[nodiscard]] int find_ith_char(char c, int count) const {
        if (count == 0) {
            return 0;
        }

        int tracking_count = 0;
        for (auto it = begin(); it != end(); ++it) {
            if (*it == c) {
                ++tracking_count;
                if (count == tracking_count) {
                    return std::distance(begin(), it);
                }
            }
        }
        return -1;
    }

    // Iterators
    iterator begin() noexcept { return iterator(this, bufferStart); }
    iterator end() noexcept { return iterator(this, bufferEnd); }
    const_iterator begin() const noexcept { return const_iterator(this, bufferStart); }
    const_iterator end() const noexcept { return const_iterator(this, bufferEnd); }
    const_iterator cbegin() const noexcept { return const_iterator(this, bufferStart); }
    const_iterator cend() const noexcept { return const_iterator(this, bufferEnd); }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(iterator(this, (gapEnd == bufferEnd) ? gapStart : bufferEnd));
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(iterator(this, (gapStart == bufferStart) ? gapEnd : bufferStart));
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(
            const_iterator(this, (gapEnd == bufferEnd) ? gapStart : bufferEnd));
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(
            const_iterator(this, (gapStart == bufferStart) ? gapEnd : bufferStart));
    }
    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(this->cbegin());
    }
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(
            const_iterator(this, (gapStart == bufferStart) ? gapEnd : bufferStart));
    }

    // Capacity
    [[nodiscard]] constexpr bool empty() const noexcept { return bufferStart == gapStart; }
    [[nodiscard]] constexpr size_type size() const noexcept {
        return bufferEnd - bufferStart - (gapEnd - gapStart);
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return bufferEnd - bufferStart; }

    constexpr void reserve(int new_cap) {
        if (new_cap > bufferEnd - bufferStart) {
            pointer new_mem = allocator_type().allocate(new_cap);
            std::uninitialized_copy_n(bufferStart, capacity(), new_mem);

            allocator_type().deallocate(bufferStart, capacity());

            gapStart = new_mem + (gapStart - bufferStart);
            gapEnd = new_mem + (gapEnd - bufferStart);
            bufferEnd = new_mem + new_cap;
            bufferStart = new_mem;
        }
    }

    [[nodiscard]] constexpr unsigned int line_count() const noexcept {
        if (bufferStart == gapStart && bufferEnd == gapEnd) {
            return 0;
        }
        int newlines = std::count(begin(), end(), '\n');
        if (back() != '\n') {
            newlines++;
        }
        return newlines;
    }

    // Modifiers
    constexpr void clear() noexcept {
        std::destroy_n(bufferStart, capacity());
        gapStart = bufferStart;
        gapEnd = bufferEnd;
    }

    constexpr void insert(iterator pos, const_reference value) {
        std::uninitialized_copy_n(pos.ptr, gapStart - pos.ptr, pos.ptr + 1);
        *pos.ptr = value;

        ++gapStart;

        if (static_cast<size_type>(gapEnd - gapStart) == 0) {
            resize(capacity() * 2);
        }
    }

    constexpr void insert(iterator pos, const std::string_view value) {
        std::uninitialized_copy_n(pos.ptr, gapStart - pos.ptr, pos.ptr + value.size());
        std::copy(value.begin(), value.end(), pos.ptr);
        gapStart += value.size();

        if (static_cast<size_type>(gapEnd - gapStart) == 0) {
            resize(capacity() * 2);
        }
    }

    constexpr void erase(iterator pos) {
        std::copy(pos.ptr + 1, gapStart, pos.ptr);
        --gapStart;
    }

    constexpr void erase(iterator pos, size_type count) {
        std::copy(pos.ptr + count, gapStart, pos.ptr);
        gapStart -= count;
    }

    constexpr void push_back(const T& value) {
        *gapStart = value;
        gapStart++;
        if (gapStart == gapEnd) {
            resize(capacity() * 2);
        }
    }
    constexpr void pop_back() {
        gapStart -= 1;
        *gapStart = value_type();
    }

    constexpr void resize(const size_type count) {
        if (count <= size()) {
            // TODO: Resize to be smaller?
            return;
        }

        int prefix = gapStart - bufferStart;
        int suffix = bufferEnd - gapEnd;
        int old_cap = capacity();
        pointer new_mem = allocator_type().allocate(count);

        pointer gap_start_point = std::uninitialized_move(begin(), begin() + prefix, new_mem);
        pointer buf_end_point =
            std::uninitialized_move(end() - suffix, end(), (new_mem + count - suffix));

        allocator_type().deallocate(bufferStart, old_cap);

        bufferEnd = buf_end_point;
        gapStart = gap_start_point;
        gapEnd = buf_end_point - suffix;
        bufferStart = new_mem;

        for (pointer i = gapStart; i < gapEnd; i++) {
            *i = value_type();
        }
    }

   private:
    pointer bufferStart = nullptr;
    pointer gapStart = nullptr;
    pointer gapEnd = nullptr;
    pointer bufferEnd = nullptr;
};

#endif  // GAPVECTOR_H
