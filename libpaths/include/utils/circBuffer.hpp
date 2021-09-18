#pragma once

namespace Utils {

template<typename T, typename Alloc = std::allocator<T>>
class CircularBuffer {
    typedef Alloc AllocType;
    typedef std::allocator_traits<AllocType> AllocTraits;

    AllocType allocator;

    size_t frontIdx{0}; //read head, when buffer has data, always points to some valid data, invalid if empty
    size_t backIdx{0}; //write head, always points to invalid data

    const size_t bufferSize;
    char *buffer{nullptr};

    void CreateBuffer() noexcept {
        buffer = new char[sizeof(value_type) * bufferSize];
    }

    inline void AssertNotFull() { if (full()) throw std::out_of_range("can't push new items, the circular buffer is full"); }

    inline void AssertNotEmpty() { if (empty()) throw std::out_of_range("can't retrieve items, the circular buffer is empty"); }

    [[nodiscard]] size_t Inc(size_t v) const noexcept { return (v + 1) % max_size(); }

    [[nodiscard]] size_t Add(size_t v, size_t amount) const noexcept { return (v + amount) % max_size(); }

    [[nodiscard]] size_t Dec(size_t v) const noexcept { return (v - 1 + max_size()) % max_size(); }

    [[nodiscard]] bool InBounds(size_t idx) const noexcept { return backIdx >= frontIdx ? (idx >= frontIdx) && (idx < backIdx) : (idx <= frontIdx) && (idx > backIdx); }

  public:
    typedef T value_type;
    typedef typename AllocTraits::pointer pointer;
    typedef typename AllocTraits::const_pointer const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    /*typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;*/

    explicit CircularBuffer(size_t size) requires std::is_nothrow_default_constructible_v<AllocType>
      : allocator(AllocType())
        , bufferSize(size) {
        CreateBuffer();
    }

    explicit CircularBuffer(size_t size, const AllocType &allocator)
      : allocator(allocator)
        , bufferSize(size) {
        CreateBuffer();
    }

    ~CircularBuffer() {
        for (; frontIdx != backIdx; frontIdx = Inc(frontIdx)) {
            GetAtIdx(frontIdx)->~value_type();
        }

        delete[](buffer);
    }

    [[nodiscard]] size_type max_size() const noexcept { return bufferSize; }

    [[nodiscard]] size_type capacity() const noexcept { return max_size(); }

    [[nodiscard]] size_type size() const noexcept {
        return backIdx >= frontIdx ? backIdx - frontIdx : max_size() - (frontIdx - backIdx);
    }

    [[nodiscard]] bool full() const noexcept { return Inc(backIdx) == frontIdx; }

    [[nodiscard]] bool empty() const noexcept { return backIdx == frontIdx; }

    void push_back(value_type &&v) {
        AssertNotFull();
        new(GetRawAtIdx(backIdx)) value_type(std::forward<value_type>(v));
        backIdx = Inc(backIdx);
    }

    void push_back(const_reference v) {
        AssertNotFull();
        new(GetRawAtIdx(backIdx)) value_type(v);
        backIdx = Inc(backIdx);
    }

    template<typename ...Args>
    reference emplace_back(Args &&...args) {
        push_back(std::move(value_type(std::forward<Args>(args)...)));
        return back();
    }

    void pop_front() {
        AssertNotEmpty();
        GetAtIdx(frontIdx)->~value_type();
        frontIdx = Inc(frontIdx);
    }

    void pop_back() {
        AssertNotEmpty();
        GetAtIdx(backIdx)->~value_type();
        backIdx = Dec(backIdx);
    }

    reference front() { return *GetAtIdx(frontIdx); }

    reference back() { return *GetAtIdx(backIdx); }

    reference operator[](size_t idx) noexcept {
        auto actualIdx = Add(frontIdx, idx);
        return *GetAtIdx(idx);
    }

    reference at(size_t idx) {
        if (!InBounds(idx)) throw std::out_of_range(fmt::format("at called with index {} which is out of bounds (back: {}, front: {})", idx, frontIdx, backIdx));
        return this->operator[](idx);
    }

    const_reference operator[](size_t idx) const noexcept { return const_cast<const_reference>(const_cast<CircularBuffer<T> *>(this)->operator[](idx)); }

    const_reference at(size_t idx) const { return const_cast<const_reference>(const_cast<CircularBuffer<T> *>(this)->at(idx)); }

    [[nodiscard]] void *GetRawAtIdx(size_t idx) noexcept {
        return static_cast<void *>(&buffer[idx * sizeof(value_type)]);
    }

    [[nodiscard]] pointer GetAtIdx(size_t idx) noexcept {
        return static_cast<pointer>(GetRawAtIdx(idx));
    }
};

}
