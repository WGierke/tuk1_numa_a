// From https://github.com/ReidAtcheson/numaallocator

/* (C) Copyright Reid Atcheson 2016.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */
#include <numa.h>

template <class T> class NumaAlloc {
public:
const int node;

typedef T         value_type;
typedef size_t    size_type;
typedef ptrdiff_t difference_type;
typedef T*        pointer;
typedef const T*  const_pointer;
typedef T&        reference;
typedef const T&  const_reference;

template< class U > struct rebind { typedef NumaAlloc<U> other; };

typedef std::true_type propagate_on_container_move_assignment;
typedef std::true_type is_always_equal;

NumaAlloc(int node = 0) noexcept : node{node} {};
template <typename TOther>
NumaAlloc(const NumaAlloc<TOther> &other) noexcept : node{other.node} {};

size_type max_size() const { return size_t(-1); }

T* allocate (size_t num) {
    auto ret = numa_alloc_onnode(num * sizeof(T), node);
    if (!ret)
        throw std::bad_alloc();
    return reinterpret_cast<T*>(ret);
}

void deallocate (T* p, size_t num) noexcept {
    numa_free(p, num * sizeof(T));
}
};

template <class T1, class T2>
bool operator== (const NumaAlloc<T1> &,
            const NumaAlloc<T2> &) noexcept {
    return true;
}

template <class T1, class T2>
bool operator!= (const NumaAlloc<T1> &,
            const NumaAlloc<T2> &) noexcept {
    return false;
}
