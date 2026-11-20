// =============================================================================
//  Stack.h  --  Member 2
//  Linked-node LIFO stack (template).  No std::stack.
//  push / pop / peek: O(1).
//  Used by: Undo feature (Member 2), DFS and path printing (Member 3).
// =============================================================================
#ifndef ROUTEWISE_STACK_H
#define ROUTEWISE_STACK_H

template <typename T>
class Stack {
private:
    struct Node {
        T     data;
        Node* next;
        Node(const T& d, Node* n) : data(d), next(n) {}
    };
    Node* topNode;
    int   count;

    Stack(const Stack&);
    Stack& operator=(const Stack&);

public:
    Stack() : topNode(nullptr), count(0) {}
    ~Stack() { clear(); }

    void push(const T& v) { topNode = new Node(v, topNode); ++count; }

    bool pop(T& out) {
        if (!topNode) return false;
        Node* n = topNode;
        out = n->data;
        topNode = n->next;
        delete n;
        --count;
        return true;
    }

    bool peek(T& out) const {
        if (!topNode) return false;
        out = topNode->data;
        return true;
    }

    bool isEmpty() const { return count == 0; }
    int  size()    const { return count; }

    void clear() {
        T tmp;
        while (pop(tmp)) {}
    }

    // Top -> bottom
    template <typename Func>
    void forEach(Func f) const {
        for (Node* cur = topNode; cur; cur = cur->next) f(cur->data);
    }
};

#endif
