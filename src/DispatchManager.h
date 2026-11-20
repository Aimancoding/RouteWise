// =============================================================================
//  DispatchManager.h  --  Member 2
//  Ties together: normal Queue, urgent Heap, undo Stack and history list.
//
//  Parcel life-cycle:  Registered -> Pending -> Dispatched -> Delivered
//  Every transition is pushed on the undo stack together with the data needed
//  to reverse it exactly (previous status + where the parcel was).
// =============================================================================
#ifndef ROUTEWISE_DISPATCHMANAGER_H
#define ROUTEWISE_DISPATCHMANAGER_H

#include <iostream>
#include <string>
#include "Parcel.h"
#include "Queue.h"
#include "Stack.h"
#include "UrgentHeap.h"
#include "HistoryLog.h"

enum ActionType {
    ACT_REQUEST_NORMAL,
    ACT_REQUEST_URGENT,
    ACT_DISPATCH,
    ACT_DELIVER
};

struct UndoAction {
    ActionType     type;
    Parcel*        parcel;
    DeliveryStatus prevStatus;
    bool           fromHeap;     // for ACT_DISPATCH: did it come from the heap?
    UndoAction() : type(ACT_REQUEST_NORMAL), parcel(nullptr),
                   prevStatus(STATUS_REGISTERED), fromHeap(false) {}
};

class DispatchManager {
private:
    Queue<Parcel*> normalQueue;
    UrgentHeap     urgentHeap;
    Stack<UndoAction> undoStack;
    DeliveryHistory   history;
    long nextArrival;
    int  deliveredCount;

    void record(ActionType t, Parcel* p, DeliveryStatus prev, bool fromHeap,
                const char* actionName) {
        UndoAction ua;
        ua.type = t; ua.parcel = p; ua.prevStatus = prev; ua.fromHeap = fromHeap;
        undoStack.push(ua);
        history.append(p->getTrackingID(), actionName, prev, p->getStatus());
    }

public:
    DispatchManager() : nextArrival(0), deliveredCount(0) {}

    bool requestNormal(Parcel* p, std::string& msg) {
        if (p->getStatus() != STATUS_REGISTERED) {
            msg = "Parcel is already " + std::string(statusToString(p->getStatus())) +
                  "; only Registered parcels can be requested.";
            return false;
        }
        DeliveryStatus prev = p->getStatus();
        p->setArrivalOrder(++nextArrival);
        p->setStatus(STATUS_PENDING);
        normalQueue.enqueue(p);
        record(ACT_REQUEST_NORMAL, p, prev, false, "Normal request");
        msg = "Added to normal delivery queue.";
        return true;
    }

    bool requestUrgent(Parcel* p, std::string& msg) {
        if (p->getStatus() != STATUS_REGISTERED) {
            msg = "Parcel is already " + std::string(statusToString(p->getStatus())) +
                  "; only Registered parcels can be requested.";
            return false;
        }
        DeliveryStatus prev = p->getStatus();
        p->setArrivalOrder(++nextArrival);
        p->setStatus(STATUS_PENDING);
        urgentHeap.push(p);
        record(ACT_REQUEST_URGENT, p, prev, false, "Urgent request");
        msg = "Added to urgent priority heap.";
        return true;
    }

    // Urgent heap first (highest priority, then earliest arrival), then the
    // normal queue (FIFO).  Returns nullptr if nothing is pending.
    Parcel* processNext() {
        Parcel* p = nullptr;
        bool fromHeap = false;
        if (urgentHeap.pop(p)) fromHeap = true;
        else if (!normalQueue.dequeue(p)) return nullptr;

        DeliveryStatus prev = p->getStatus();
        p->setStatus(STATUS_DISPATCHED);
        record(ACT_DISPATCH, p, prev, fromHeap, "Dispatched");
        return p;
    }

    bool markDelivered(Parcel* p, std::string& msg) {
        if (p->getStatus() != STATUS_DISPATCHED) {
            msg = "Only Dispatched parcels can be delivered (current status: " +
                  std::string(statusToString(p->getStatus())) + ").";
            return false;
        }
        DeliveryStatus prev = p->getStatus();
        p->setStatus(STATUS_DELIVERED);
        ++deliveredCount;
        record(ACT_DELIVER, p, prev, false, "Delivered");
        msg = "Parcel marked as delivered.";
        return true;
    }

    // Reverses the most recent status-changing operation.
    bool undo(std::string& msg) {
        UndoAction ua;
        if (!undoStack.pop(ua)) { msg = "Nothing to undo."; return false; }
        Parcel* p = ua.parcel;
        const std::string id = p->getTrackingID();

        switch (ua.type) {
            case ACT_REQUEST_NORMAL:
                normalQueue.remove(p);
                --nextArrival;
                msg = "Undid normal request for " + id;
                break;
            case ACT_REQUEST_URGENT:
                urgentHeap.remove(p);
                --nextArrival;
                msg = "Undid urgent request for " + id;
                break;
            case ACT_DISPATCH:
                if (ua.fromHeap) urgentHeap.push(p);
                else             normalQueue.pushFront(p);
                msg = "Undid dispatch of " + id + " (back in " +
                      (ua.fromHeap ? "urgent heap)" : "normal queue)");
                break;
            case ACT_DELIVER:
                --deliveredCount;
                msg = "Undid delivery of " + id + " (back to Dispatched)";
                break;
        }
        p->setStatus(ua.prevStatus);
        history.removeLast();
        return true;
    }

    // ---- information ----
    int normalCount()  const { return normalQueue.size(); }
    int urgentCount()  const { return urgentHeap.size(); }
    int pendingCount() const { return normalQueue.size() + urgentHeap.size(); }
    int undoDepth()    const { return undoStack.size(); }
    int delivered()    const { return deliveredCount; }
    int historySize()  const { return history.size(); }

    void displayPending(std::ostream& os = std::cout) const {
        os << "--- Urgent deliveries (dispatch order) ---\n";
        if (urgentHeap.isEmpty()) os << "  (none)\n";
        int i = 1;
        urgentHeap.forEachInOrder([&](Parcel* p) { os << "  " << i++ << ". "; p->display(os); });

        os << "--- Normal deliveries (FIFO order) ---\n";
        if (normalQueue.isEmpty()) os << "  (none)\n";
        i = 1;
        normalQueue.forEach([&](Parcel* p) { os << "  " << i++ << ". "; p->display(os); });
    }

    void displayHistory(std::ostream& os = std::cout) const { history.display(os); }
};

#endif
