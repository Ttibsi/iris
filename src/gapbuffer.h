#ifndef GAPBUFFER_H
#define GAPBUFFER_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// Gap buffer data structure implementation: https://en.wikipedia.org/wiki/Gap_buffer
class Gapbuffer {
   public:
    // member types
    using value_type = char;
    using allocator_type = std::allocator<char>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = char&;
    using const_reference = const char&;
    using pointer = std::allocator_traits<std::allocator<char>>::pointer;
    using const_pointer = std::allocator_traits<std::allocator<char>>::const_pointer;

   private:
    // Iterator built from: https://medium.com/p/fc5b994462c6#90c4
    // https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
    // https://medium.com/@joao_vaz/c-iterators-and-implementing-your-own-custom-one-a-primer-72f1506e5d71

    template <typename pointer_type>
    class IteratorTemplate {
       public:
        static const bool is_const = std::is_const_v<std::remove_pointer_t<pointer_type>>;

        using value_type = typename std::conditional<is_const, const char, char>::type;
        using gapbuffer_ptr_type =
            typename std::conditional<is_const, const Gapbuffer*, Gapbuffer*>::type;
        using difference_type = std::ptrdiff_t;
        using pointer = pointer_type;
        using reference = value_type&;
        using iterator_category = std::random_access_iterator_tag;

        explicit IteratorTemplate() = default;
        explicit IteratorTemplate(pointer_type input_ptr, gapbuffer_ptr_type gb_ptr)
            : ptr(input_ptr), buf_ptr(gb_ptr) {}

        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }
        reference operator[](difference_type n) const { return *(ptr + n); }

        IteratorTemplate& operator++() {
            ptr++;
            if (ptr == buf_ptr->gapStart) {
                ptr = buf_ptr->gapEnd;
            }
            return *this;
        }

        // postfix increment
        IteratorTemplate operator++(int) {
            IteratorTemplate tmp = *this;
            ++(*this);
            if (ptr == buf_ptr->gapStart) {
                ptr = buf_ptr->gapEnd;
            }
            return tmp;
        }

        IteratorTemplate& operator--() {
            ptr--;
            if (ptr == buf_ptr->gapStart) {
                ptr = buf_ptr->gapEnd;
            }
            return *this;
        }

        // postfix decrement
        IteratorTemplate operator--(int) {
            IteratorTemplate tmp = *this;
            --(*this);
            if (ptr == buf_ptr->gapStart) {
                ptr = buf_ptr->gapEnd;
            }
            return tmp;
        }

        IteratorTemplate operator+(const difference_type other) const {
            pointer new_ptr = ptr + other;

            if (ptr <= buf_ptr->gapStart && new_ptr > buf_ptr->gapStart) {
                new_ptr += (buf_ptr->gapEnd - buf_ptr->gapStart);
            } else if (ptr >= buf_ptr->gapEnd && new_ptr < buf_ptr->gapEnd) {
                new_ptr -= (buf_ptr->gapEnd - buf_ptr->gapStart);
            }

            return IteratorTemplate(ptr + other, buf_ptr);
        }

        friend IteratorTemplate operator+(
            const difference_type value,
            const IteratorTemplate& other) {
            return other + value;
        }

        friend difference_type operator+(const IteratorTemplate& a, const IteratorTemplate& b) {
            return *a + *b;
        }

        IteratorTemplate operator-(const difference_type other) const {
            pointer new_ptr = ptr - other;

            if (ptr >= buf_ptr->gapEnd && new_ptr < buf_ptr->gapEnd) {
                new_ptr -= (buf_ptr->gapEnd - buf_ptr->gapStart);
            } else if (ptr <= buf_ptr->gapStart && new_ptr > buf_ptr->gapStart) {
                new_ptr += (buf_ptr->gapEnd - buf_ptr->gapStart);
            }

            return IteratorTemplate(ptr - other, buf_ptr);
        }

        friend IteratorTemplate operator-(
            const difference_type value,
            const IteratorTemplate& other) {
            return other - value;
        }

        friend difference_type operator-(
            const IteratorTemplate& self,
            const IteratorTemplate& other) {
            difference_type distance = self.ptr - other.ptr;

            // Adjust for gap
            if (self.ptr > self.buf_ptr->gapStart && other.ptr <= self.buf_ptr->gapStart) {
                distance -= (self.buf_ptr->gapEnd - self.buf_ptr->gapStart);
            } else if (self.ptr <= self.buf_ptr->gapStart && other.ptr > self.buf_ptr->gapStart) {
                distance += (self.buf_ptr->gapEnd - self.buf_ptr->gapStart);
            }

            return distance;
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

       private:
        pointer_type ptr;
        gapbuffer_ptr_type buf_ptr;
    };

   public:
    // Iterator member types
    using iterator = IteratorTemplate<pointer>;
    using const_iterator = IteratorTemplate<const_pointer>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Constructors
    constexpr explicit Gapbuffer() {
        bufferStart = allocator_type().allocate(32);
        bufferEnd = std::uninitialized_value_construct_n(bufferStart, 32);
        gapStart = bufferStart;
        gapEnd = bufferEnd;

        static_assert(std::random_access_iterator<iterator>);
        static_assert(std::random_access_iterator<const_iterator>);
        static_assert(std::ranges::random_access_range<Gapbuffer>);
    }

    constexpr explicit Gapbuffer(const size_type length) {
        if (length < 2) {
            throw std::runtime_error("Cannot construct gapbuffer with capacity < 2");
        }

        bufferStart = allocator_type().allocate(length);
        bufferEnd = std::uninitialized_value_construct_n(bufferStart, length);
        gapStart = bufferStart;
        gapEnd = bufferEnd;

        static_assert(std::random_access_iterator<iterator>);
        static_assert(std::random_access_iterator<const_iterator>);
        static_assert(std::ranges::random_access_range<Gapbuffer>);
    }

    constexpr explicit Gapbuffer(std::string_view str) {
        bufferStart = allocator_type().allocate(str.size() + 8);

        const auto [_, final_destination] =
            std::uninitialized_move_n(str.begin(), str.size(), bufferStart);
        gapStart = final_destination;
        gapEnd = gapStart + 8;
        bufferEnd = gapEnd;

        static_assert(std::random_access_iterator<iterator>);
        static_assert(std::random_access_iterator<const_iterator>);
        static_assert(std::ranges::random_access_range<Gapbuffer>);
    }

    template <typename InputIt>
    constexpr explicit Gapbuffer(InputIt begin, InputIt end) {
        const unsigned int len = std::distance(begin, end);

        bufferStart = allocator_type().allocate(len + 8);

        auto [_, final_destination] = std::uninitialized_move_n(begin, len, bufferStart);
        gapStart = final_destination;
        gapEnd = gapStart + 8;
        bufferEnd = gapEnd;

        static_assert(std::random_access_iterator<iterator>);
        static_assert(std::random_access_iterator<const_iterator>);
        static_assert(std::ranges::random_access_range<Gapbuffer>);
    }

    constexpr Gapbuffer(std::initializer_list<char> lst) {
        bufferStart = allocator_type().allocate(lst.size());
        const auto [_, final_destination] =
            std::uninitialized_move_n(lst.begin(), lst.size(), bufferStart);
        gapStart = final_destination;
        gapEnd = gapStart + 8;
        bufferEnd = gapEnd;

        static_assert(std::random_access_iterator<iterator>);
        static_assert(std::random_access_iterator<const_iterator>);
        static_assert(std::ranges::random_access_range<Gapbuffer>);
    }

    // Copy Constructor
    constexpr Gapbuffer(const Gapbuffer& other) {
        bufferStart = allocator_type().allocate(other.capacity());
        bufferEnd = std::uninitialized_copy_n(other.bufferStart, other.capacity(), bufferStart);
        gapStart = bufferStart + (other.gapStart - other.bufferStart);
        gapEnd = bufferStart + (other.gapEnd - other.bufferStart);
    }

    // Copy Assignment
    constexpr Gapbuffer& operator=(const Gapbuffer& other) {
        if (this != &other) {
            bufferStart = allocator_type().allocate(other.capacity());

            gapStart = std::uninitialized_copy_n(other.bufferStart, other.capacity(), bufferStart);

            gapEnd = bufferStart + (other.gapEnd - other.bufferStart);
            bufferEnd = bufferStart + other.capacity();
        }
        return *this;
    }

    // Move Constructor
    constexpr Gapbuffer(Gapbuffer&& other) {
        bufferStart = allocator_type().allocate(other.capacity());

        const auto [_, moved] =
            std::uninitialized_move_n(other.bufferStart, other.capacity(), bufferStart);
        bufferEnd = moved;
        gapStart = bufferStart + (other.gapStart - other.bufferStart);
        gapEnd = bufferStart + (other.gapEnd - other.bufferStart);

        other.bufferStart = nullptr;
        other.gapStart = nullptr;
        other.gapEnd = nullptr;
        other.bufferEnd = nullptr;

        allocator_type().deallocate(other.bufferStart, other.capacity());
    }

    // Move Assignment Operator
    constexpr Gapbuffer& operator=(Gapbuffer&& other) {
        if (this != &other) {
            std::destroy_n(bufferStart, capacity());
            allocator_type().deallocate(bufferStart, capacity());

            bufferStart = std::exchange(other.bufferStart, {});
            gapStart = std::exchange(other.gapStart, {});
            gapEnd = std::exchange(other.gapEnd, {});
            bufferEnd = std::exchange(other.bufferEnd, {});
        }
        return *this;
    }

    ~Gapbuffer() {
        std::destroy_n(bufferStart, capacity());
        allocator_type().deallocate(bufferStart, capacity());
    }

    // Operator Overloads
    friend std::ostream& operator<<(std::ostream& os, const Gapbuffer& buf) {
        os << "[";
        for (auto p = buf.bufferStart; p < buf.bufferEnd; p++) {
            if (p >= buf.gapStart && p < buf.gapEnd) {
                os << " ";
            } else {
                os << *p;
            }
        }
        os << "]";

        return os;
    }

    [[nodiscard]] constexpr reference operator[](const size_type& loc) {
        if (loc > size()) {
            throw std::out_of_range("Out of bounds indexing");
        }

        if (loc < static_cast<size_type>(gapStart - bufferStart)) {
            return *(bufferStart + loc);
        } else {
            return *(gapEnd + (loc - (gapStart - bufferStart)));
        }
    }

    [[nodiscard]] constexpr const_reference operator[](const size_type& loc) const {
        if (loc > size()) {
            throw std::out_of_range("Out of bounds indexing");
        }

        if (loc < static_cast<size_type>(gapStart - bufferStart)) {
            return *(bufferStart + loc);
        } else {
            return *(gapEnd + (loc - (gapStart - bufferStart)));
        }
    }

    [[nodiscard]] bool operator==(const Gapbuffer& other) const noexcept {
        if (size() != other.size()) {
            return false;
        }
        if (to_str() != other.to_str()) {
            return false;
        }
        return true;
    }

    [[nodiscard]] bool operator!=(const Gapbuffer& other) const noexcept = default;

    // Element Access
    // TODO: When I upgrade to c++23, look into `deducing this` and `std::forward_like` to
    // use metaprogramming to need only one function for each function type here instead
    // of two
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
        if (gapStart == bufferStart && gapEnd == bufferEnd) {
            throw std::out_of_range("Accessing front element in an empty Gapbuffer");
        }

        if (bufferStart == gapStart) {
            return *gapEnd;
        }

        return *bufferStart;
    }

    [[nodiscard]] constexpr const_reference front() const {
        if (gapStart == bufferStart && gapEnd == bufferEnd) {
            throw std::out_of_range("Accessing front element in an empty Gapbuffer");
        }

        if (bufferStart == gapStart) {
            return *gapEnd;
        }

        return *bufferStart;
    }

    [[nodiscard]] constexpr reference back() {
        if (gapStart == bufferStart && gapEnd == bufferEnd) {
            throw std::out_of_range("Accessing back element in an empty Gapbuffer");
        }

        if (bufferEnd == gapEnd) {
            return *(gapStart - 1);
        }

        return *(bufferEnd - 1);
    }

    [[nodiscard]] constexpr const_reference back() const {
        if (gapStart == bufferStart && gapEnd == bufferEnd) {
            throw std::out_of_range("Accessing back element in an empty Gapbuffer");
        }

        if (bufferEnd == gapEnd) {
            return *(gapStart - 1);
        }

        return *(bufferEnd - 1);
    }

    [[nodiscard]] const std::string to_str() const noexcept {
        std::string ret;
        ret.reserve(size());
        ret.append(bufferStart, (gapStart - bufferStart));
        ret.append(gapEnd, (bufferEnd - gapEnd));
        return ret;
    }

    [[nodiscard]] std::string line(size_type pos) const {
        if (pos > size()) {
            throw std::out_of_range("index out of range");
        }
        if (empty()) {
            throw std::runtime_error("Cannot pull line from empty gapbuffer");
        }

        auto range = std::ranges::subrange(begin(), end());
        auto line_start =
            std::ranges::find(std::ranges::reverse_view(range | std::views::take(pos)), '\n')
                .base();

        if (&*line_start == bufferStart && &*line_start == gapStart) {
            // If we find the gap at the start
            line_start += gap_size();
        } else if (&*line_start >= gapStart && &*line_start < gapStart) {
            // if line_start lies within the gap
            line_start += gap_size();
        }

        auto line_end = std::ranges::find(std::ranges::drop_view(range, pos), '\n');
        if (line_end != end()) {
            line_end++;
        }
        if (&*line_end >= gapStart && &*line_end < gapEnd) {
            line_end += gap_size();
        }

        // return std::string(line_start, line_end);
        std::string ret;
        for (auto it = line_start; it < line_end; ++it) {
            ret.push_back(*it);
        }

        return ret;
    }

    [[nodiscard]] int find(char c, int count = 1) const {
        if (count == 0) {
            return 0;
        }

        int tracking_count = 0;
        for (auto it = begin(); it != end(); ++it) {
            if (*it == c) {
                tracking_count++;
                if (count == tracking_count) {
                    if (&*it < gapEnd && &*it >= gapStart) {
                        it += gap_size();
                    }
                    return std::distance(begin(), it);
                }
            }
        }

        return -1;
    }

    // Iterators
    iterator begin() noexcept { return iterator(bufferStart, this); }
    iterator end() noexcept { return iterator(bufferEnd, this); }
    const_iterator begin() const noexcept { return const_iterator(bufferStart, this); }
    const_iterator end() const noexcept { return const_iterator(bufferEnd, this); }
    const_iterator cbegin() const noexcept { return const_iterator(bufferStart, this); }
    const_iterator cend() const noexcept { return const_iterator(bufferEnd, this); }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(iterator((gapEnd == bufferEnd) ? gapStart : bufferEnd, this));
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(iterator((gapStart == bufferStart) ? gapEnd : bufferStart, this));
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(
            const_iterator((gapEnd == bufferEnd) ? gapStart : bufferEnd, this));
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(
            const_iterator((gapStart == bufferStart) ? gapEnd : bufferStart, this));
    }
    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(this->cbegin());
    }
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(
            const_iterator((gapStart == bufferStart) ? gapEnd : bufferStart, this));
    }

    // Capacity
    [[nodiscard]] constexpr bool empty() const noexcept {
        return bufferStart == gapStart && bufferEnd == gapEnd;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return bufferEnd - bufferStart - (gapEnd - gapStart);
    }

    [[nodiscard]] constexpr size_type gap_size() const noexcept { return gapEnd - gapStart; }
    [[nodiscard]] constexpr size_type capacity() const noexcept { return bufferEnd - bufferStart; }
    [[nodiscard]] constexpr size_type pos() const noexcept { return gapStart - bufferStart; }

    constexpr void reserve(size_type new_cap) {
        if (new_cap > capacity()) {
            pointer new_mem = allocator_type().allocate(new_cap);
            pointer new_end = std::uninitialized_value_construct_n(new_mem, new_cap);

            const auto [_, lhs_buf] =
                std::uninitialized_move_n(bufferStart, gapStart - bufferStart, new_mem);
            gapStart = lhs_buf;

            std::size_t rhs_size = bufferEnd - gapEnd;
            std::uninitialized_move_n(gapEnd, rhs_size, new_end - rhs_size);

            bufferStart = new_mem;
            bufferEnd = new_end;
            gapStart = lhs_buf;
            gapEnd = bufferEnd - rhs_size;
        }
    }

    [[nodiscard]] constexpr unsigned int line_count() const noexcept {
        if (bufferStart == gapStart && bufferEnd == gapEnd) {
            return 0;
        }

        int newlines = std::count(bufferStart, gapStart, '\n');
        newlines += std::count(gapEnd, bufferEnd, '\n');

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

    constexpr void insert(const std::string_view value) {
        for (auto&& c : value) {
            push_back(c);
        }
    }

    // Intentionally discardable
    constexpr std::string erase(size_type count) {
        std::string ret = "";

        for (unsigned int i = 0; i < count; i++) {
            ret.push_back(pop_back());
        }

        return ret;
    }

    constexpr void push_back(const char& value) {
        *gapStart = value;
        gapStart++;
        if (gapStart == gapEnd) {
            reserve(capacity() * 2);
        }
    }

    // Intentionally discardable
    constexpr char pop_back() {
        if (gapStart == bufferStart) {
            throw std::out_of_range("Buffer is empty");
        }

        const char ret = *(gapStart - 1);
        std::destroy_at(gapStart);
        gapStart -= 1;
        return ret;
    }

    constexpr void advance() {
        if (gap_size() == 0) {
            return;
        }
        std::optional<char> c = std::nullopt;

        if (gapEnd < bufferEnd) {
            c = *gapEnd;
            std::destroy_at(gapEnd);
        }

        gapEnd++;
        if (c.has_value()) {
            *gapStart = c.value();
        }
        gapStart++;
    }

    constexpr void retreat() {
        const unsigned int prev_gap = gap_size();
        if (bufferStart == gapStart) {
            return;
        }

        std::optional<char> c = std::nullopt;

        if (gapStart > bufferStart) {
            c = *(gapStart - 1);
            std::destroy_at(gapStart - 1);
        }

        gapStart--;
        gapEnd--;
        if (c.has_value()) {
            *gapEnd = c.value();
        }

        assert(gap_size() == prev_gap);
    }

   private:
    pointer bufferStart;
    pointer gapStart;   // One space past the last value in the left half
    pointer gapEnd;     // Pointing to the first value in the right half
    pointer bufferEnd;  // One space past the last value in the right half
};

namespace std {
    template <>
    inline std::size_t size(const Gapbuffer& buf) noexcept {
        return buf.size();
    }

    template <>
    inline std::ptrdiff_t ssize(const Gapbuffer& buf) noexcept {
        return static_cast<std::ptrdiff_t>(buf.size());
    }
}  // namespace std

#endif  // GAPBUFFER_H
