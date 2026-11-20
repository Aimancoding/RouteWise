// =============================================================================
//  Queue.h  --  Member 2
//  Linked-node FIFO queue (template).  No std::queue.
//  enqueue / dequeue / front: O(1).  remove(value): O(n).
//  Used by: normal delivery requests (Member 2) and BFS (Member 3).
// =============================================================================
#ifndef ROUTEWISE_QUEUE_H
#define ROUTEWISE_QUEUE_H

template <typename T>
class Queue {
private:
    struct Node {
        T     data;
        Node* next;
        explicit Node(const T& d) : data(d), next(nullptr) {}
    };
    Node* head;
    Node* tail;
    int   count;

    Queue(const Queue&);
    Queue& operator=(const Queue&);

public:
    Queue() : head(nullptr), tail(nullptr), count(0) {}
    ~Queue() { clear(); }

    void enqueue(const T& v) {
        Node* n = new Node(v);
        if (tail) tail->next = n; else head = n;
        tail = n;
        ++count;
    }

    // Puts an element back at the FRONT (used by Undo of a dispatch).
    void pushFront(const T& v) {
        Node* n = new Node(v);
        n->next = head;
        head = n;
        if (!tail) tail = n;
        ++count;
    }

    bool dequeue(T& out) {
        if (!head) return false;
        Node* n = head;
        out = n->data;
        head = n->next;
        if (!head) tail = nullptr;
        delete n;
        --count;
        return true;
    }

    bool front(T& out) const {
        if (!head) return false;
        out = head->data;
        return true;
    }

    // Removes the first element equal to v (used by Undo of a request).
    bool remove(const T& v) {
        Node* prev = nullptr;
        for (Node* cur = head; cur; prev = cur, cur = cur->next) {
            if (cur->data == v) {
                if (prev) prev->next = cur->next; else head = cur->next;
                if (cur == tail) tail = prev;
                delete cur;
                --count;
                return true;
            }
        }
        return false;
    }

    bool isEmpty() const { return count == 0; }
    int  size()    const { return count; }

    void clear() {
        T tmp;
        while (dequeue(tmp)) {}
    }

    // Front -> back.  q.forEach([&](const T& x){ ... });
    template <typename Func>
    void forEach(Func f) const {
        for (Node* cur = head; cur; cur = cur->next) f(cur->data);
    }
};

#endif
