#ifndef LIST_H_
#define LIST_H_

template <typename T>
class List
{
public:
    struct Node
    {
        T *value;
        Node *next;

        Node(T *value, Node *next = nullptr) : value(value), next(next) {}
    };

    void append(T *value);
    void push_back(T *value);
    unsigned size() const { return cnt; }

    void begin() { iter = first; }
    bool end() const { return iter != nullptr; }
    void next()
    {
        if (iter)
            iter = iter->next;
    }
    T *get() const { return iter; }
    T *get(unsigned index) const;

private:
    Node *first = nullptr, *last = nullptr, *iter = nullptr;
    unsigned cnt = 0;
};


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

#endif
