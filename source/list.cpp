#include "list.hpp"

template <typename T>
void List<T>::append(T *value) {
    last = (first ? last->next : first) = new Node(value);
    ++cnt;
}

template <typename T>
void List<T>::push_back(T *value) {
    first = new Node(value, first);
    if (!last) last = first;
    ++cnt;
}

template <typename T>
T* List<T>::get(unsigned index) const {
    if(index >= size()) return nullptr;

    T *curr;
    for(curr = first; curr && index > 0; curr = curr->next, --index);

    return curr;
}
