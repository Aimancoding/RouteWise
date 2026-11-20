// =============================================================================
//  TreeComparison.h  --  Member 1
//  Menu option 16: "Compare BST and AVL Performance".
//  Builds a BST and an AVL from the parcels currently in the hash table
//  (in the order the hash table yields them) and also from the same parcels
//  in SORTED order (worst case for a BST), then prints the heights.
// =============================================================================
#ifndef ROUTEWISE_TREE_COMPARISON_H
#define ROUTEWISE_TREE_COMPARISON_H

#include <iostream>
#include <string>
#include "Parcel.h"
#include "ParcelHashTable.h"
#include "ParcelBST.h"
#include "ParcelAVL.h"

inline int minPossibleHeight(int n) {          // ceil(log2(n+1))
    int hgt = 0;
    while ((1 << hgt) - 1 < n) ++hgt;
    return hgt;
}

// Core helper: prints a comparison for an array of parcel pointers.
inline void compareTreesOnArray(Parcel** arr, int n, const std::string& label,
                                std::ostream& os = std::cout) {
    ParcelBST bst;
    ParcelAVL avl;
    for (int i = 0; i < n; ++i) { bst.insert(arr[i]); avl.insert(arr[i]); }

    os << "--- " << label << " (" << n << " parcels) ---\n";
    os << "  BST height : " << bst.height() << "\n";
    os << "  AVL height : " << avl.height()
       << "   (AVL rotations: " << avl.getRotationCount()
       << ", single " << avl.getSingleRotations()
       << ", double " << avl.getDoubleRotations() << ")\n";
    os << "  Optimal (perfectly balanced) height: " << minPossibleHeight(n) << "\n";
    os << "  AVL balanced: " << (avl.isBalanced() ? "yes" : "NO") << "\n";
    if (avl.height() > 0)
        os << "  BST is " << (double)bst.height() / avl.height()
           << "x taller than AVL\n";
}

// Menu option 16 entry point.
inline void compareBSTandAVL(const ParcelHashTable& table, std::ostream& os = std::cout) {
    int n = table.size();
    if (n == 0) { os << "No parcels registered - nothing to compare.\n"; return; }

    Parcel** arr = new Parcel*[n];
    int k = 0;
    table.forEach([&](Parcel* p) { arr[k++] = p; });

    compareTreesOnArray(arr, n, "Hash-table order", os);

    // Insertion sort by tracking ID -> worst case for the plain BST
    for (int i = 1; i < n; ++i) {
        Parcel* key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j]->getTrackingID() > key->getTrackingID()) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
    compareTreesOnArray(arr, n, "Sorted insertion order (BST worst case)", os);

    delete[] arr;
}

#endif
