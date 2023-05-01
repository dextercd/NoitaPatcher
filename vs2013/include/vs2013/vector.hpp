#ifndef NP_VS13_VECTOR_HPP
#define NP_VS13_VECTOR_HPP

#include <cstdint>

namespace vs13 {

template<class T>
struct vector {
    T* begin_;
    T* end_;
    T* capacity_end_;

    T* data() { return begin_; }

    T& front() { return *begin_; }
    T& back() { return *(end_ - 1); }

    std::size_t size() const { return end_ - begin_; }
    bool empty() const { return size() == 0; }

    T* begin() { return begin_; }
    T* end() { return end_; }
};

} // vs13::

#endif // Header guard
