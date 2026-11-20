// =============================================================================
//  ParcelHashTable.h  --  Member 1
//  Custom hash table, open addressing with QUADRATIC PROBING, keyed by
//  tracking ID (string).  Average O(1) insert / search / remove.
//
//  * Capacity is always a prime and the load factor is kept <= 0.5, which
//    guarantees quadratic probing always finds a free slot.
//  * Deleted slots use tombstones (so probe chains are never broken).
//  * The table OWNS the Parcel objects (deletes them in destructor).
// =============================================================================
#ifndef ROUTEWISE_PARCEL_HASHTABLE_H
#define ROUTEWISE_PARCEL_HASHTABLE_H

#include <iostream>
#include <string>
#include "Parcel.h"

class ParcelHashTable {
private:
    enum SlotState { EMPTY, OCCUPIED, DELETED };

    struct Slot {
        Parcel*   parcel;
        SlotState state;
        Slot() : parcel(nullptr), state(EMPTY) {}
    };

    Slot* table;
    int   capacity;
    int   count;        // OCCUPIED slots
    int   tombstones;   // DELETED slots
    int   lastProbes;   // probes used by the most recent operation
    long  totalProbes;  // for statistics
    long  totalOps;

    ParcelHashTable(const ParcelHashTable&);            // non-copyable
    ParcelHashTable& operator=(const ParcelHashTable&);

    static bool isPrime(int n) {
        if (n < 2) return false;
        for (int i = 2; (long)i * i <= n; ++i) if (n % i == 0) return false;
        return true;
    }
    static int nextPrime(int n) {
        while (!isPrime(n)) ++n;
        return n;
    }

    // Polynomial string hash (base 31), reduced modulo capacity.
    unsigned long hashFunction(const std::string& key) const {
        unsigned long h = 0;
        for (size_t i = 0; i < key.length(); ++i)
            h = h * 31 + (unsigned char)key[i];
        return h % capacity;
    }

    void rehash(int newCapacity) {
        Slot* old    = table;
        int   oldCap = capacity;

        capacity   = nextPrime(newCapacity);
        table      = new Slot[capacity];
        count      = 0;
        tombstones = 0;

        for (int i = 0; i < oldCap; ++i)
            if (old[i].state == OCCUPIED) placeNoCheck(old[i].parcel);
        delete[] old;
    }

    // Insert without duplicate check (used while rehashing)
    void placeNoCheck(Parcel* p) {
        unsigned long h = hashFunction(p->getTrackingID());
        for (int i = 0; i < capacity; ++i) {
            int idx = (int)((h + (unsigned long)i * i) % capacity);
            if (table[idx].state != OCCUPIED) {
                table[idx].parcel = p;
                table[idx].state  = OCCUPIED;
                ++count;
                return;
            }
        }
    }

    // Returns slot index of key, or -1.  Sets lastProbes.
    int findIndex(const std::string& key) {
        unsigned long h = hashFunction(key);
        lastProbes = 0;
        for (int i = 0; i < capacity; ++i) {
            int idx = (int)((h + (unsigned long)i * i) % capacity);
            ++lastProbes;
            if (table[idx].state == EMPTY) break;
            if (table[idx].state == OCCUPIED &&
                table[idx].parcel->getTrackingID() == key) {
                return idx;
            }
        }
        return -1;
    }

    void record() { totalProbes += lastProbes; ++totalOps; }

public:
    explicit ParcelHashTable(int initialCapacity = 11)
        : table(nullptr), capacity(nextPrime(initialCapacity < 5 ? 5 : initialCapacity)),
          count(0), tombstones(0), lastProbes(0), totalProbes(0), totalOps(0) {
        table = new Slot[capacity];
    }

    ~ParcelHashTable() {
        for (int i = 0; i < capacity; ++i)
            if (table[i].state == OCCUPIED) delete table[i].parcel;
        delete[] table;
    }

    // Takes ownership of p on success. Returns false (and does NOT take
    // ownership) if the tracking ID already exists.
    bool insert(Parcel* p) {
        if (!p) return false;
        if (findIndex(p->getTrackingID()) != -1) { record(); return false; }

        if ((count + tombstones + 1) * 2 > capacity) rehash(capacity * 2);

        unsigned long h = hashFunction(p->getTrackingID());
        int firstDeleted = -1;
        lastProbes = 0;
        for (int i = 0; i < capacity; ++i) {
            int idx = (int)((h + (unsigned long)i * i) % capacity);
            ++lastProbes;
            if (table[idx].state == DELETED && firstDeleted == -1) firstDeleted = idx;
            if (table[idx].state == EMPTY) {
                if (firstDeleted != -1) { idx = firstDeleted; --tombstones; }
                table[idx].parcel = p;
                table[idx].state  = OCCUPIED;
                ++count;
                record();
                return true;
            }
        }
        if (firstDeleted != -1) {   // table full of tombstones/occupied
            table[firstDeleted].parcel = p;
            table[firstDeleted].state  = OCCUPIED;
            --tombstones; ++count;
            record();
            return true;
        }
        return false;
    }

    // Returns the Parcel* (still owned by the table) or nullptr.
    Parcel* search(const std::string& trackingID) {
        int idx = findIndex(trackingID);
        record();
        return idx == -1 ? nullptr : table[idx].parcel;
    }

    // Removes AND deletes the parcel. Warning: any other structure still
    // holding this Parcel* (BST, AVL, queue...) must drop it first.
    bool remove(const std::string& trackingID) {
        int idx = findIndex(trackingID);
        record();
        if (idx == -1) return false;
        delete table[idx].parcel;
        table[idx].parcel = nullptr;
        table[idx].state  = DELETED;
        --count; ++tombstones;
        return true;
    }

    int    size()          const { return count; }
    int    getCapacity()   const { return capacity; }
    double loadFactor()    const { return (double)count / capacity; }
    int    getLastProbes() const { return lastProbes; }
    double averageProbes() const { return totalOps ? (double)totalProbes / totalOps : 0.0; }

    // Visit every parcel (for building BST/AVL, displaying records, etc.)
    // Usage: table.forEach([&](Parcel* p){ ... });
    template <typename Func>
    void forEach(Func f) const {
        for (int i = 0; i < capacity; ++i)
            if (table[i].state == OCCUPIED) f(table[i].parcel);
    }

    void displayAll(std::ostream& os = std::cout) const {
        if (count == 0) { os << "No parcel records.\n"; return; }
        for (int i = 0; i < capacity; ++i)
            if (table[i].state == OCCUPIED) table[i].parcel->display(os);
    }

    // Shows the internal table layout (useful for demonstrating probing)
    void displayTable(std::ostream& os = std::cout) const {
        os << "Hash table (capacity " << capacity << ", size " << count
           << ", load " << loadFactor() << ")\n";
        for (int i = 0; i < capacity; ++i) {
            os << "  [" << i << "] ";
            if (table[i].state == EMPTY)         os << "empty\n";
            else if (table[i].state == DELETED)  os << "<deleted>\n";
            else os << table[i].parcel->getTrackingID() << "\n";
        }
    }
};

#endif
