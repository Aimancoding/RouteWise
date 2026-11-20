// =============================================================================
//  UrgentHeap.h  --  Member 2
//  Binary MAX-heap (array based) of Parcel* for urgent deliveries.
//  Order: higher priority first; equal priority -> smaller arrivalOrder first.
//  push / pop: O(log n)   top: O(1)   remove(p): O(n)
//  Parcels are NOT owned by the heap.
// =============================================================================
#ifndef ROUTEWISE_URGENTHEAP_H
#define ROUTEWISE_URGENTHEAP_H

#include "Parcel.h"

class UrgentHeap {
private:
    Parcel** a;
    int      count;
    int      capacity;

    UrgentHeap(const UrgentHeap&);
    UrgentHeap& operator=(const UrgentHeap&);

    // true if x must come out BEFORE y
    static bool before(const Parcel* x, const Parcel* y) {
        if (x->getPriority() != y->getPriority())
            return x->getPriority() > y->getPriority();
        return x->getArrivalOrder() < y->getArrivalOrder();
    }

    void grow() {
        int newCap = capacity * 2;
        Parcel** bigger = new Parcel*[newCap];
        for (int i = 0; i < count; ++i) bigger[i] = a[i];
        delete[] a;
        a = bigger;
        capacity = newCap;
    }

    void swapAt(int i, int j) { Parcel* t = a[i]; a[i] = a[j]; a[j] = t; }

    void bubbleUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (before(a[i], a[parent])) { swapAt(i, parent); i = parent; }
            else break;
        }
    }

    void percolateDown(int i) {
        for (;;) {
            int l = 2 * i + 1, r = 2 * i + 2, best = i;
            if (l < count && before(a[l], a[best])) best = l;
            if (r < count && before(a[r], a[best])) best = r;
            if (best == i) break;
            swapAt(i, best);
            i = best;
        }
    }

public:
    UrgentHeap() : a(new Parcel*[16]), count(0), capacity(16) {}
    ~UrgentHeap() { delete[] a; }

    void push(Parcel* p) {
        if (count == capacity) grow();
        a[count] = p;
        bubbleUp(count);
        ++count;
    }

    bool pop(Parcel*& out) {
        if (count == 0) return false;
        out = a[0];
        a[0] = a[--count];
        if (count > 0) percolateDown(0);
        return true;
    }

    bool top(Parcel*& out) const {
        if (count == 0) return false;
        out = a[0];
        return true;
    }

    bool remove(Parcel* p) {
        for (int i = 0; i < count; ++i) {
            if (a[i] == p) {
                a[i] = a[--count];
                if (i < count) { bubbleUp(i); percolateDown(i); }
                return true;
            }
        }
        return false;
    }

    bool isEmpty() const { return count == 0; }
    int  size()    const { return count; }

    // Visits parcels in dispatch order without changing the heap.
    template <typename Func>
    void forEachInOrder(Func f) const {
        UrgentHeap tmp;
        for (int i = 0; i < count; ++i) tmp.push(a[i]);
        Parcel* p;
        while (tmp.pop(p)) f(p);
    }
};

#endif
