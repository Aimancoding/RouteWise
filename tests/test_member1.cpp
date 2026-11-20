// =============================================================================
//  member1_test.cpp -- stand-alone test/demo for Member 1's modules.
//  Build:  g++ -std=c++11 -Wall -Wextra -o member1_test member1_test.cpp
//  (This file is NOT part of the final project; main() will come from the
//   integrated application.)
// =============================================================================
#include <iostream>
#include <string>
#include <cstdlib>
#include "Customer.h"
#include "Parcel.h"
#include "ParcelHashTable.h"
#include "ParcelBST.h"
#include "ParcelAVL.h"
#include "TreeComparison.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::cout << "  FAIL: " #cond " (line " << __LINE__ << ")\n"; ++failures; } } while (0)

static std::string makeID(int i) {
    std::string s = std::to_string(i);
    while (s.length() < 4) s = "0" + s;
    return "TRK" + s;
}

int main() {
    std::cout << "=== Customers ===\n";
    CustomerRegistry customers;
    CHECK(customers.addCustomer(1, "Ali Khan", "0300-1111111", "House 5, Gulshan"));
    CHECK(customers.addCustomer(2, "Sara Ahmed", "0321-2222222", "Flat 9, DHA"));
    CHECK(!customers.addCustomer(1, "Dup", "0", "x"));
    CHECK(customers.findCustomer(2) != nullptr);
    CHECK(customers.findCustomer(99) == nullptr);
    customers.displayAll();

    std::cout << "\n=== Hash table (small, forces rehash + collisions) ===\n";
    ParcelHashTable table(5);
    const int N = 200;
    for (int i = 1; i <= N; ++i)
        CHECK(table.insert(new Parcel(makeID(i), 1 + i % 2, "Warehouse", "Airport", 1.5 + i % 7, 1 + i % 5)));
    CHECK(table.size() == N);
    Parcel* dup = new Parcel(makeID(10), 1, "A", "B", 1, 1);
    CHECK(!table.insert(dup));
    delete dup;                                    // not owned on failure

    for (int i = 1; i <= N; ++i) CHECK(table.search(makeID(i)) != nullptr);
    CHECK(table.search("NOPE") == nullptr);
    CHECK(table.loadFactor() <= 0.5);
    std::cout << "  capacity " << table.getCapacity() << ", load " << table.loadFactor()
              << ", avg probes " << table.averageProbes() << "\n";

    // remove + tombstone reuse
    CHECK(table.remove(makeID(50)));
    CHECK(!table.remove(makeID(50)));
    CHECK(table.search(makeID(50)) == nullptr);
    CHECK(table.search(makeID(51)) != nullptr);
    CHECK(table.insert(new Parcel(makeID(50), 2, "W", "H", 2, 3)));
    CHECK(table.search(makeID(50)) != nullptr);
    CHECK(table.size() == N);

    std::cout << "\n=== BST / AVL correctness ===\n";
    ParcelBST bst;
    ParcelAVL avl;
    table.forEach([&](Parcel* p) { bst.insert(p); avl.insert(p); });
    CHECK(bst.size() == N && avl.size() == N);
    CHECK(avl.isBalanced());
    CHECK(avl.height() <= 1.45 * minPossibleHeight(N) + 1);

    // in-order must be sorted in both trees
    std::string prev; bool sortedB = true, sortedA = true;
    bst.forEachInOrder([&](Parcel* p){ if (p->getTrackingID() < prev) sortedB = false; prev = p->getTrackingID(); });
    prev = "";
    avl.forEachInOrder([&](Parcel* p){ if (p->getTrackingID() < prev) sortedA = false; prev = p->getTrackingID(); });
    CHECK(sortedB && sortedA);

    CHECK(bst.search(makeID(77)) == avl.search(makeID(77)));
    CHECK(avl.search("NOPE") == nullptr);
    CHECK(!avl.insert(avl.search(makeID(77))));    // duplicate rejected

    for (int i = 1; i <= N; i += 2) { CHECK(bst.remove(makeID(i))); CHECK(avl.remove(makeID(i))); }
    CHECK(bst.size() == N / 2 && avl.size() == N / 2);
    CHECK(avl.isBalanced());
    CHECK(avl.search(makeID(1)) == nullptr);
    CHECK(avl.search(makeID(2)) != nullptr);

    std::cout << "\n=== The 4 AVL rotation cases ===\n";
    {
        Parcel a("C", 1, "x", "y", 1, 1), b("B", 1, "x", "y", 1, 1), c("A", 1, "x", "y", 1, 1);
        ParcelAVL t1; t1.insert(&a); t1.insert(&b); t1.insert(&c);     // LL
        CHECK(t1.height() == 2 && t1.getSingleRotations() == 1);
        Parcel d("A", 1, "x", "y", 1, 1), e("B", 1, "x", "y", 1, 1), f("C", 1, "x", "y", 1, 1);
        ParcelAVL t2; t2.insert(&d); t2.insert(&e); t2.insert(&f);     // RR
        CHECK(t2.height() == 2 && t2.getSingleRotations() == 1);
        Parcel g("C", 1, "x", "y", 1, 1), i("A", 1, "x", "y", 1, 1), j("B", 1, "x", "y", 1, 1);
        ParcelAVL t3; t3.insert(&g); t3.insert(&i); t3.insert(&j);     // LR
        CHECK(t3.height() == 2 && t3.getDoubleRotations() == 1);
        Parcel k("A", 1, "x", "y", 1, 1), l("C", 1, "x", "y", 1, 1), m("B", 1, "x", "y", 1, 1);
        ParcelAVL t4; t4.insert(&k); t4.insert(&l); t4.insert(&m);     // RL
        CHECK(t4.height() == 2 && t4.getDoubleRotations() == 1);
    }

    std::cout << "\n=== Comparison (menu option 16) ===\n";
    compareBSTandAVL(table);

    std::cout << "\n=== Small tree picture + traversals ===\n";
    {
        ParcelAVL demo;
        Parcel p1("TRK0003", 1, "W", "A", 1, 1), p2("TRK0001", 1, "W", "A", 1, 1),
               p3("TRK0002", 1, "W", "A", 1, 1), p4("TRK0005", 1, "W", "A", 1, 1),
               p5("TRK0004", 1, "W", "A", 1, 1);
        demo.insert(&p1); demo.insert(&p2); demo.insert(&p3); demo.insert(&p4); demo.insert(&p5);
        demo.printStructure();
        std::cout << "In-order:\n";   demo.inorder();
        std::cout << "Pre-order:\n";  demo.preorder();
        std::cout << "Post-order:\n"; demo.postorder();
    }

    std::cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n";
    return failures;   // table's destructor frees all parcels
}
