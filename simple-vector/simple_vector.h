#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>

using namespace std;

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity) :
    capacity_(capacity) {}

    size_t GetCapacity() {
        return capacity_;
    }

private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) :
    items_(size), size_(size), capacity_(size) {
        std::fill(items_.Get(), items_.Get() + size_, Type());
    }

    SimpleVector(size_t size, const Type& value) :
    items_(size), size_(size), capacity_(size) {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }

    SimpleVector(std::initializer_list<Type> init) 
    : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) 
    : items_(other.size_), size_(other.size_), capacity_(other.capacity_) {
        copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) noexcept
    : items_(move(other.items_)), size_(other.size_), capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector(ReserveProxyObj object) :
    items_(object.GetCapacity()), size_(0), capacity_(object.GetCapacity()){}

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_.Get()[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_.Get()[index];
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            items_ = move(rhs.items_);
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            rhs.size_ = 0;
            rhs.capacity_ = 0;
        }
        return *this;
    }

    Iterator begin() noexcept {
        return size_ == 0 ? nullptr : items_.Get();
    }

    Iterator end() noexcept {
        return size_ == 0 ? nullptr : items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return size_ == 0 ? nullptr : items_.Get();
    }

    ConstIterator end() const noexcept {
        return size_ == 0 ? nullptr : items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return size_ == 0 ? nullptr : items_.Get();
    }

    ConstIterator cend() const noexcept {
        return size_ == 0 ? nullptr : items_.Get() + size_;
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw out_of_range("Index is out of range");
        }
        return items_.Get()[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw out_of_range("Index is out of range");
        }
        return items_.Get()[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= capacity_) {
            if (new_size > size_) {
                for (size_t i = size_; i < new_size; i++) {
                    items_.Get()[i] = Type();
                }
            }
            size_ = new_size;
        } else {
            Reserve(max(new_size, 2*capacity_));
            for (size_t i = size_; i < new_size; i++) {
                    items_.Get()[i] = Type();
            }
            size_ = new_size;
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : 2*capacity_);
        }
        items_.Get()[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : 2*capacity_);
        }
        items_.Get()[size_] = move(item);
        ++size_;
    }
    
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0) ? 1 : 2*capacity_;
            ArrayPtr<Type> new_items(new_capacity);
            move(begin(), begin() + index, new_items.Get());
            new_items.Get()[index] = value;
            move(begin() + index, end(), new_items.Get() + index + 1);
            items_.swap(new_items);
            capacity_ = new_capacity;
        } else {
            for (size_t i = size_; i > index; --i) {
                items_.Get()[i] = move(items_.Get()[i-1]);
            }
            items_.Get()[index] = value;
        }
        ++size_;
        return begin() + index;
    }

        Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0) ? 1 : 2*capacity_;
            ArrayPtr<Type> new_items(new_capacity);
            move(begin(), begin() + index, new_items.Get());
            new_items.Get()[index] = move(value);
            move(begin() + index, end(), new_items.Get() + index + 1);
            items_.swap(new_items);
            capacity_ = new_capacity;
        } else {
            for (size_t i = size_; i > index; --i) {
                items_.Get()[i] = move(items_.Get()[i-1]);
            }
            items_.Get()[index] = move(value);
        }
        ++size_;
        return begin() + index;
    }

    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();
        for (size_t i = index; i < size_ - 1; ++i) {
            items_.Get()[i] = move(items_.Get()[i + 1]);
        }
        --size_;
        return begin() + index;
        
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 