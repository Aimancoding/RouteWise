// =============================================================================
//  ParcelBST.h  --  Member 1
//  Standard (unbalanced) Binary Search Tree, ordered by tracking ID.
//  Nodes hold Parcel* (NOT owned - the hash table owns parcels).
//  insert / search / remove: O(h)  (O(log n) average, O(n) worst case)
// =============================================================================
#ifndef ROUTEWISE_PARCEL_BST_H
#define ROUTEWISE_PARCEL_BST_H

#include <iostream>
#include <string>
#include "Parcel.h"

class ParcelBST {
private:
    struct Node {
        Parcel* parcel;
        Node*   left;
        Node*   right;
        explicit Node(Parcel* p) : parcel(p), left(nullptr), right(nullptr) {}
    };

    Node* root;
    int   count;

    ParcelBST(const ParcelBST&);
    ParcelBST& operator=(const ParcelBST&);

    static const std::string& keyOf(Node* n) { return n->parcel->getTrackingID(); }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    static int height(Node* n) {            // empty tree = 0
        if (!n) return 0;
        int l = height(n->left), r = height(n->right);
        return 1 + (l > r ? l : r);
    }

    Node* insert(Node* n, Parcel* p, bool& inserted) {
        if (!n) { inserted = true; return new Node(p); }
        if (p->getTrackingID() < keyOf(n))      n->left  = insert(n->left,  p, inserted);
        else if (p->getTrackingID() > keyOf(n)) n->right = insert(n->right, p, inserted);
        // equal key: duplicate, ignored
        return n;
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
                return child;
            }
            Node* succ = minNode(n->right);
            n->parcel = succ->parcel;
            bool dummy = false;
            n->right = remove(n->right, keyOf(n), dummy);
        }
        return n;
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
        os << keyOf(n) << "\n";
        printStructure(n->left, depth + 1, os);
    }

public:
    ParcelBST() : root(nullptr), count(0) {}
    ~ParcelBST() { destroy(root); }

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
    int  height() const { return height(root); }
    bool empty()  const { return root == nullptr; }

    void inorder  (std::ostream& os = std::cout) const { inorder(root, os); }
    void preorder (std::ostream& os = std::cout) const { preorder(root, os); }
    void postorder(std::ostream& os = std::cout) const { postorder(root, os); }

    // Sorted visit: tree.forEachInOrder([&](Parcel* p){ ... });
    template <typename Func>
    void forEachInOrder(Func f) const { inorderVisit(root, f); }

    // Sideways picture of the tree (root on the left)
    void printStructure(std::ostream& os = std::cout) const { printStructure(root, 0, os); }
};

#endif
