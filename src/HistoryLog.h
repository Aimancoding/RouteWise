// =============================================================================
//  HistoryLog.h  --  Member 2
//  Doubly linked list recording every status-changing operation.
//  append: O(1)   removeLast (for Undo): O(1)   display: O(n)
// =============================================================================
#ifndef ROUTEWISE_HISTORYLOG_H
#define ROUTEWISE_HISTORYLOG_H

#include <iostream>
#include <string>
#include "Parcel.h"

struct HistoryEntry {
    long           seq;
    std::string    trackingID;
    std::string    action;
    DeliveryStatus oldStatus;
    DeliveryStatus newStatus;
};

class DeliveryHistory {
private:
    struct Node {
        HistoryEntry e;
        Node* prev;
        Node* next;
        Node() : prev(nullptr), next(nullptr) {}
    };
    Node* head;
    Node* tail;
    int   count;
    long  seqCounter;

    DeliveryHistory(const DeliveryHistory&);
    DeliveryHistory& operator=(const DeliveryHistory&);

public:
    DeliveryHistory() : head(nullptr), tail(nullptr), count(0), seqCounter(0) {}
    ~DeliveryHistory() { while (removeLast()) {} }

    void append(const std::string& trackingID, const std::string& action,
                DeliveryStatus oldS, DeliveryStatus newS) {
        Node* n = new Node();
        n->e.seq = ++seqCounter;
        n->e.trackingID = trackingID;
        n->e.action = action;
        n->e.oldStatus = oldS;
        n->e.newStatus = newS;
        n->prev = tail;
        if (tail) tail->next = n; else head = n;
        tail = n;
        ++count;
    }

    bool removeLast() {
        if (!tail) return false;
        Node* n = tail;
        tail = n->prev;
        if (tail) tail->next = nullptr; else head = nullptr;
        delete n;
        --count;
        --seqCounter;
        return true;
    }

    int size() const { return count; }

    int countWithNewStatus(DeliveryStatus s) const {
        int c = 0;
        for (Node* cur = head; cur; cur = cur->next)
            if (cur->e.newStatus == s) ++c;
        return c;
    }

    void display(std::ostream& os = std::cout) const {
        if (!head) { os << "Delivery history is empty.\n"; return; }
        for (Node* cur = head; cur; cur = cur->next)
            os << "#" << cur->e.seq << " [" << cur->e.trackingID << "] "
               << cur->e.action << ": " << statusToString(cur->e.oldStatus)
               << " -> " << statusToString(cur->e.newStatus) << "\n";
    }
};

#endif
