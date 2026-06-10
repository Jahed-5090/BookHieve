#pragma once
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <cstddef>

template<typename T>
class Array {
private:
    T* data_;
    std::size_t sz_;
    std::size_t cap_;

    void grow(std::size_t minCap) {
        std::size_t newCap = cap_ ? cap_ * 2 : 4;
        if (newCap < minCap) newCap = minCap;
        T* newData = new T[newCap];
        for (std::size_t i = 0; i < sz_; ++i) newData[i] = data_[i];
        delete[] data_;
        data_ = newData;
        cap_ = newCap;
    }

public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    Array() : data_(nullptr), sz_(0), cap_(0) {}
    Array(const Array& o) : data_(nullptr), sz_(0), cap_(0) {
        if (o.sz_) {
            data_ = new T[o.sz_];
            for (std::size_t i = 0; i < o.sz_; ++i) data_[i] = o.data_[i];
            sz_ = cap_ = o.sz_;
        }
    }
    Array(Array&& o) noexcept : data_(o.data_), sz_(o.sz_), cap_(o.cap_) { o.data_ = nullptr; o.sz_ = o.cap_ = 0; }

    Array(std::initializer_list<T> il) : data_(nullptr), sz_(0), cap_(0) {
        reserve(il.size());
        for (const auto& v : il) push_back(v);
    }

    template<typename InputIt>
    Array(InputIt first, InputIt last) : data_(nullptr), sz_(0), cap_(0) {
        auto dist = std::distance(first, last);
        if (dist > 0) {
            reserve((std::size_t)dist);
            for (auto it = first; it != last; ++it) push_back(*it);
        }
    }

    ~Array() { delete[] data_; }

    Array& operator=(const Array& o) {
        if (this == &o) return *this;
        delete[] data_;
        data_ = nullptr; sz_ = cap_ = 0;
        if (o.sz_) {
            data_ = new T[o.sz_];
            for (std::size_t i = 0; i < o.sz_; ++i) data_[i] = o.data_[i];
            sz_ = cap_ = o.sz_;
        }
        return *this;
    }

    Array& operator=(Array&& o) noexcept {
        if (this == &o) return *this;
        delete[] data_;
        data_ = o.data_; sz_ = o.sz_; cap_ = o.cap_;
        o.data_ = nullptr; o.sz_ = o.cap_ = 0;
        return *this;
    }

    std::size_t size() const { return sz_; }
    bool empty() const { return sz_ == 0; }

    void reserve(std::size_t n) {
        if (n <= cap_) return;
        T* newData = new T[n];
        for (std::size_t i = 0; i < sz_; ++i) newData[i] = data_[i];
        delete[] data_;
        data_ = newData; cap_ = n;
    }

    void push_back(const T& v) {
        if (sz_ >= cap_) grow(sz_ + 1);
        data_[sz_++] = v;
    }

    void pop_back() { if (sz_) --sz_; }

    T& operator[](std::size_t i) { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    iterator begin() { return data_; }
    iterator end() { return data_ + sz_; }
    const_iterator begin() const { return data_; }
    const_iterator end() const { return data_ + sz_; }

    void clear() { sz_ = 0; }

    T& back() { return data_[sz_ - 1]; }
    const T& back() const { return data_[sz_ - 1]; }

    T* data() { return data_; }
    const T* data() const { return data_; }
};
