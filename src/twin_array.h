#ifndef TWIN_ARRAY_H
#define TWIN_ARRAY_H

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

// TODO: Static asserts and exceptions

template <typename T> class TwinArray {
  public:
    // member types
    using value_type = T;
    using allocator_type = std::allocator<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T &;
    using const_reference = const T &;
    using pointer = std::allocator_traits<std::allocator<T>>::pointer;
    using const_pointer =
        std::allocator_traits<std::allocator<T>>::const_pointer;

  private:
    template <typename ptr_type> class IteratorTemplate {
      public:
        static constexpr bool is_const =
            std::is_const_v<std::remove_pointer_t<ptr_type>>;

        using value_type = std::conditional<is_const, const T, T>::type;
        using container_ptr =
            std::conditional<is_const, const TwinArray *, TwinArray *>::type;
        using difference_type = std::ptrdiff_t;
        using pointer = ptr_type;
        using reference = value_type &;
        using iterator_category = std::random_access_iterator_tag;

        IteratorTemplate() = default;
        IteratorTemplate(container_ptr parent, std::size_t idx)
            : parent(parent), index(idx) {}

        // Dereference element pointed to by the iterator
        reference operator*() const {
            if (index < parent->lhs_size) {
                // Accessing lhs normally
                return parent->lhs[index];
            } else {
                // Accessing rhs in reverse
                std::size_t rhs_index = index - parent->lhs_size;
                return parent->rhs[parent->rhs_size - rhs_index - 1];
            }
        }

        pointer operator->() const { return &**this; }

        // Random access
        reference operator[](difference_type n) const { return *(*this + n); }

        // Arithmetic operators
        IteratorTemplate &operator++() {
            if (index < parent->lhs_size + parent->rhs_size) {
                ++index;
            }
            return *this;
        }

        IteratorTemplate operator++(int) {
            IteratorTemplate tmp = *this;
            ++(*this);
            return tmp;
        }

        IteratorTemplate &operator--() {
            if (index > 0) {
                --index;
            }
            return *this;
        }

        IteratorTemplate operator--(int) {
            IteratorTemplate tmp = *this;
            --(*this);
            return tmp;
        }

        IteratorTemplate operator+(difference_type n) const {
            std::size_t new_index =
                std::min(index + n, parent->lhs_size + parent->rhs_size);
            return IteratorTemplate(parent, new_index);
        }

        IteratorTemplate operator-(std::size_t n) const {
            std::size_t new_index = (n > index) ? 0 : index - n;
            return IteratorTemplate(parent, new_index);
        }

        difference_type operator-(const IteratorTemplate &other) const {
            return static_cast<difference_type>(index) -
                   static_cast<difference_type>(other.index);
        }

        IteratorTemplate &operator+=(difference_type n) {
            index += std::min(n, parent->lhs_size + parent->rhs_size);
            return *this;
        }

        IteratorTemplate &operator-=(difference_type n) {
            if (n > index) {
                index = 0;
            } else {
                index -= n;
            }
            return *this;
        }

        // Comparison operators (using <=> for C++20 compatibility)
        auto operator<=>(const IteratorTemplate &other) const = default;

      private:
        container_ptr parent = nullptr;
        std::size_t index = 0;
    };

  public:
    // Iterator member types
    using iterator = IteratorTemplate<pointer>;
    using const_iterator = IteratorTemplate<const_pointer>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Constructors
    constexpr explicit TwinArray(const std::size_t len = 32)
        : lhs(std::make_unique<T[]>(len)), rhs(std::make_unique<T[]>(len)),
          lhs_size(0), rhs_size(0), capacity(len) {}

    template <typename InputIt>
    constexpr explicit TwinArray(InputIt begin, InputIt end)
        : TwinArray(std::distance(begin, end) + 8) {
        lhs_size = std::distance(begin, end);
        std::copy(begin, end, lhs.get());
    }

    constexpr TwinArray(std::initializer_list<T> lst)
        : TwinArray(lst.size() + 8) {
        lhs_size = lst.size();
        std::copy(lst.begin(), lst.end(), lhs.get());
    }

    constexpr explicit TwinArray(std::string_view str)
        requires(std::is_same_v<T, char>)
        : TwinArray(str.size() + 8) {
        lhs_size = str.size();
        std::copy(str.begin(), str.end(), lhs.get());
    }

    // Copy Constructor
    TwinArray(const TwinArray &other)
        : lhs(std::make_unique<T[]>(other.capacity)),
          rhs(std::make_unique<T[]>(other.capacity)), lhs_size(other.lhs_size),
          rhs_size(other.rhs_size), capacity(other.capacity) {
        for (std::size_t i = 0; i < lhs_size; ++i) {
            lhs[i] = other.lhs[i];
        }

        for (std::size_t i = 0; i < rhs_size; ++i) {
            rhs[i] = other.rhs[i];
        }
    }

    // Copy Assignment Operator
    TwinArray &operator=(const TwinArray &other) {
        if (this != &other) {
            auto new_lhs = std::make_unique<T[]>(other.capacity);
            auto new_rhs = std::make_unique<T[]>(other.capacity);

            // Copy data from other
            for (std::size_t i = 0; i < other.lhs_size; ++i) {
                new_lhs[i] = other.lhs[i];
            }
            for (std::size_t i = 0; i < other.rhs_size; ++i) {
                new_rhs[i] = other.rhs[i];
            }

            lhs = std::move(new_lhs);
            rhs = std::move(new_rhs);
            lhs_size = other.lhs_size;
            rhs_size = other.rhs_size;
            capacity = other.capacity;
        }
        return *this;
    }

    // Move Constructor
    TwinArray(TwinArray &&other) noexcept
        : lhs(std::move(other.lhs)), rhs(std::move(other.rhs)),
          lhs_size(other.lhs_size), rhs_size(other.rhs_size),
          capacity(other.capacity) {
        // Reset other's state
        other.lhs_size = 0;
        other.rhs_size = 0;
        other.capacity = 0;
    }

    // Move Assignment Operator
    TwinArray &operator=(TwinArray &&other) noexcept {
        if (this != &other) {
            // Move resources
            lhs = std::move(other.lhs);
            rhs = std::move(other.rhs);
            lhs_size = other.lhs_size;
            rhs_size = other.rhs_size;
            capacity = other.capacity;

            other.lhs_size = 0;
            other.rhs_size = 0;
            other.capacity = 0;
        }
        return *this;
    }

    // Destructor
    ~TwinArray() = default;

    // Operator overloads
    friend std::ostream &operator<<(std::ostream &os, const TwinArray &buf) {
        os << "[";
        for (std::size_t i = 0; i < buf.lhs_size; i++) {
            os << buf.lhs[i] << " ";
        }
        os << "]";

        os << "[";
        for (std::size_t i = 0; i < buf.rhs_size; i++) {
            os << buf.rhs[i] << " ";
        }
        os << "]";

        return os;
    }

    [[nodiscard]] bool operator==(const TwinArray &other) const noexcept {
        return (size() == other.size()) && (to_str() == other.to_str());
    }

    [[nodiscard]] bool
    operator!=(const TwinArray &other) const noexcept = default;

    // Iterator Methods
    iterator begin() noexcept { return iterator(this, 0); }
    iterator end() noexcept { return iterator(this, lhs_size + rhs_size); }
    const_iterator begin() const noexcept { return const_iterator(this, 0); }
    const_iterator end() const noexcept {
        return const_iterator(this, lhs_size + rhs_size);
    }
    const_iterator cbegin() const noexcept { return const_iterator(this, 0); }
    const_iterator cend() const noexcept {
        return const_iterator(this, lhs_size + rhs_size);
    }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(cend());
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(cbegin());
    }
    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    // Modifiers
    void push(const T &val) {
        if (size() == capacity) {
            resize(capacity * 2);
        }
        lhs[lhs_size] = val;
        lhs_size++;
    }

    [[maybe_unused]] std::optional<T> pop() {
        if (size() == 0) {
            return {};
        }

        T ret = lhs[lhs_size - 1];
        lhs[lhs_size - 1] = T();
        lhs_size--;

        return ret;
    }

    void move_left() {
        if (lhs_size == 0) {
            return;
        }

        rhs[rhs_size] = lhs[lhs_size - 1];
        lhs[lhs_size - 1] = T();
        lhs_size--;
        rhs_size++;
    }

    void move_right() {
        if (rhs_size == 0) {
            return;
        }

        lhs[lhs_size] = rhs[rhs_size - 1];
        rhs[rhs_size - 1] = T();
        lhs_size++;
        rhs_size--;
    }

    // Element Access
    [[nodiscard]] T at(const std::size_t idx) const {
        if (idx >= size()) {
            throw std::out_of_range("index out of range");
        }

        if (idx < lhs_size) {
            return lhs[idx];
        } else if (idx < size()) {
            auto rhs_view = std::views::reverse(
                std::ranges::subrange(rhs.get(), rhs.get() + rhs_size));

            return rhs_view[idx - lhs_size];
        }

        return T();
    }

    [[nodiscard]] T peek() const { return lhs[lhs_size - 1]; }

    // TODO: See if I can replace this with algorithms
    // std::ranges::filter_view suggested
    [[nodiscard]] int find(T c, int count = 1) const {
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

    // Capacity
    [[nodiscard]] std::size_t size() const noexcept {
        return lhs_size + rhs_size;
    }
    [[nodiscard]] std::size_t total_capacity() const noexcept {
        return capacity;
    }
    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    void resize(const std::size_t new_cap) {
        auto new_lhs = std::make_unique<T[]>(new_cap);
        auto new_rhs = std::make_unique<T[]>(new_cap);

        for (std::size_t i = 0; i < lhs_size; i++) {
            new_lhs[i] = lhs[i];
        }

        for (std::size_t i = 0; i < rhs_size; i++) {
            new_rhs[i] = rhs[i];
        }

        lhs = std::move(new_lhs);
        rhs = std::move(new_rhs);
        capacity = new_cap;
    }

    // Char-only methods
    [[nodiscard]] std::string to_str() const noexcept
        requires(std::is_same_v<T, char>)
    {
        std::string ret;
        std::for_each(lhs.get(), lhs.get() + lhs_size,
                      [&](const char &c) { ret.push_back(c); });

        auto rhs_view = std::views::reverse(
            std::ranges::subrange(rhs.get(), rhs.get() + rhs_size));

        std::for_each(rhs_view.begin(), rhs_view.end(),
                      [&](const char &c) { ret.push_back(c); });

        return ret;
    }

    [[nodiscard]] std::string get_current_line() const noexcept
        requires(std::is_same_v<T, char>)
    {
        std::string ret;
        unsigned int last_idx = std::string_view(lhs.get()).find_last_of('\n');
        if (last_idx > lhs_size) {
            last_idx = 0;
        }

        for (unsigned int i = last_idx; i < lhs_size; i++) {
            ret.push_back(lhs[i]);
        }

        if (lhs[lhs_size - 1] != '\n') {
            int rhs_idx = rhs_size - 1;

            while (rhs[rhs_idx] != '\n' || rhs_idx > 0) {
                ret.push_back(rhs[rhs_idx]);
                rhs_idx--;
            }
        }

        return ret;
    }

    [[nodiscard]] char get_current_char() const noexcept
        requires(std::is_same_v<T, char>)
    {
        return lhs[lhs_size - 1];
    }

    [[nodiscard]] int curr_line_index() const noexcept
        requires(std::is_same_v<T, char>)
    {
        return std::count(lhs.get(), lhs.get() + lhs_size, '\n') + 1;
    }

    [[nodiscard]] int curr_char_index() const noexcept
        requires(std::is_same_v<T, char>)
    {
        auto last_idx = std::string_view(lhs.get()).find_last_of('\n');
        // if (last_idx > lhs_size) {
        //     last_idx = 0;
        // }
        return std::max(static_cast<int>(lhs_size - last_idx), 1);
    }

  private:
    std::unique_ptr<T[]> lhs;
    std::unique_ptr<T[]> rhs; // NOTE: rhs is stored backwards
    std::size_t lhs_size;
    std::size_t rhs_size;
    std::size_t capacity;
};

#endif // TWIN_ARRAY_H
