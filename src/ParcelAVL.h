// =============================================================================
//  ParcelAVL.h  --  Member 1
//  AVL (self-balancing BST) ordered by tracking ID.
//  Balance factor = height(left) - height(right), must stay in {-1,0,1}.
//  Rotations: LL (right), RR (left), LR (left-right), RL (right-left).
//  insert / search / remove: guaranteed O(log n).
//  Nodes hold Parcel* (NOT owned - the hash table owns parcels).
// =============================================================================
#ifndef ROUTEWISE_PARCEL_AVL_H
#define ROUTEWISE_PARCEL_AVL_H

#include <iostream>
#include <string>
#include "Parcel.h"

class ParcelAVL {
private:
    struct Node {
        Parcel* parcel;
        Node*   left;
        Node*   right;
        int     height;     // leaf = 1
        explicit Node(Parcel* p) : parcel(p), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;
    int   count;
    int   singleRotations;
    int   doubleRotations;

    ParcelAVL(const ParcelAVL&);
    ParcelAVL& operator=(const ParcelAVL&);

    static const std::string& keyOf(Node* n) { return n->parcel->getTrackingID(); }
    static int  h(Node* n) { return n ? n->height : 0; }
    static int  maxOf(int a, int b) { return a > b ? a : b; }
    static void update(Node* n) { n->height = 1 + maxOf(h(n->left), h(n->right)); }
    static int  balanceFactor(Node* n) { return n ? h(n->left) - h(n->right) : 0; }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    // ---- rotations ----
    Node* rotateRight(Node* y) {            //     y            x
        Node* x  = y->left;                 //    / \          / \.
        Node* t2 = x->right;                //   x   C   ->   A   y
        x->right = y;                       //  / \              / \.
        y->left  = t2;                      // A   t2          t2   C
        update(y); update(x);
        ++singleRotations;
        return x;
    }

    Node* rotateLeft(Node* x) {             //   x                y
        Node* y  = x->right;                //  / \              / \.
        Node* t2 = y->left;                 // A   y     ->     x   C
        y->left  = x;                       //    / \          / \.
        x->right = t2;                      //  t2   C        A   t2
        update(x); update(y);
        ++singleRotations;
        return y;
    }

    Node* rebalance(Node* n) {
        update(n);
        int bf = balanceFactor(n);

        if (bf > 1) {                                   // left heavy
            if (balanceFactor(n->left) < 0) {           // Left-Right case
                n->left = rotateLeft(n->left);
                --singleRotations; ++doubleRotations;   // count LR as ONE double rotation
            }
            return rotateRight(n);
        }
        if (bf < -1) {                                  // right heavy
            if (balanceFactor(n->right) > 0) {          // Right-Left case
                n->right = rotateRight(n->right);
                --singleRotations; ++doubleRotations;   // count RL as ONE double rotation
            }
            return rotateLeft(n);
        }
        return n;
    }

    Node* insert(Node* n, Parcel* p, bool& inserted) {
        if (!n) { inserted = true; return new Node(p); }
        if (p->getTrackingID() < keyOf(n))      n->left  = insert(n->left,  p, inserted);
        else if (p->getTrackingID() > keyOf(n)) n->right = insert(n->right, p, inserted);
        else return n;                                   // duplicate
        return rebalance(n);
    }

    static Node* minNode(Node* n) { while (n->left) n = n->left; return n; }

    Node* remove(Node* n, const std::string& key, bool& removed) {
        if (!n) return nullptr;
        if (key < keyOf(n))      n->left  = remove(n->left,  key, removed);
        else if (key > keyOf(n)) n->right = remove(n->right, key, removed);
        else {
            removed = true;
            if (!n->left || !n->right) {
                Node* child = n->left ? n->left : n->right;
                delete n;
                return child;                            // child already balanced
            }
            Node* succ = minNode(n->right);
            n->parcel = succ->parcel;
            bool dummy = false;
            n->right = remove(n->right, keyOf(n), dummy);
        }
        return rebalance(n);
    }

    static void inorder(Node* n, std::ostream& os) {
        if (!n) return;
        inorder(n->left, os); n->parcel->display(os); inorder(n->right, os);
    }
    static void preorder(Node* n, std::ostream& os) {
        if (!n) return;
        n->parcel->display(os); preorder(n->left, os); preorder(n->right, os);
    }
    static void postorder(Node* n, std::ostream& os) {
        if (!n) return;
        postorder(n->left, os); postorder(n->right, os); n->parcel->display(os);
    }

    template <typename Func>
    static void inorderVisit(Node* n, Func& f) {
        if (!n) return;
        inorderVisit(n->left, f); f(n->parcel); inorderVisit(n->right, f);
    }

    static void printStructure(Node* n, int depth, std::ostream& os) {
        if (!n) return;
        printStructure(n->right, depth + 1, os);
        for (int i = 0; i < depth; ++i) os << "      ";
        os << keyOf(n) << " (bf=" << balanceFactor(n) << ")\n";
        printStructure(n->left, depth + 1, os);
    }

    static bool checkBalanced(Node* n) {
        if (!n) return true;
        int bf = balanceFactor(n);
        if (bf < -1 || bf > 1) return false;
        return checkBalanced(n->left) && checkBalanced(n->right);
    }

public:
    ParcelAVL() : root(nullptr), count(0),
                  singleRotations(0), doubleRotations(0) {}
    ~ParcelAVL() { destroy(root); }

    bool insert(Parcel* p) {
        if (!p) return false;
        bool inserted = false;
        root = insert(root, p, inserted);
        if (inserted) ++count;
        return inserted;
    }

    Parcel* search(const std::string& trackingID) const {
        Node* cur = root;
        while (cur) {
            if (trackingID == keyOf(cur)) return cur->parcel;
            cur = (trackingID < keyOf(cur)) ? cur->left : cur->right;
        }
        return nullptr;
    }

    bool remove(const std::string& trackingID) {   // does NOT delete the Parcel
        bool removed = false;
        root = remove(root, trackingID, removed);
        if (removed) --count;
        return removed;
    }

    int  size()   const { return count; }
    int  height() const { return h(root); }
    bool empty()  const { return root == nullptr; }
    bool isBalanced() const { return checkBalanced(root); }

    int getRotationCount()   const { return singleRotations + doubleRotations; }  // LL/RR + LR/RL
    int getSingleRotations() const { return singleRotations; }
    int getDoubleRotations() const { return doubleRotations; }

    void inorder  (std::ostream& os = std::cout) const { inorder(root, os); }
    void preorder (std::ostream& os = std::cout) const { preorder(root, os); }
    void postorder(std::ostream& os = std::cout) const { postorder(root, os); }

    template <typename Func>
    void forEachInOrder(Func f) const { inorderVisit(root, f); }

    void printStructure(std::ostream& os = std::cout) const { printStructure(root, 0, os); }
};

#endif
