// Member 2 tests: Queue, Stack, UrgentHeap, DeliveryHistory, DispatchManager (+Undo)
#include <iostream>
#include <string>
#include "Parcel.h"
#include "Queue.h"
#include "Stack.h"
#include "UrgentHeap.h"
#include "HistoryLog.h"
#include "DispatchManager.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::cout << "  FAIL: " #c " (line " << __LINE__ << ")\n"; ++failures; } } while (0)

int main() {
    std::cout << "--- Queue ---\n";
    { Queue<int> q; int v;
      for (int i = 1; i <= 5; ++i) q.enqueue(i);
      CHECK(q.size() == 5 && q.dequeue(v) && v == 1 && q.dequeue(v) && v == 2);
      q.pushFront(99); CHECK(q.dequeue(v) && v == 99);
      CHECK(q.remove(4)); CHECK(!q.remove(42)); CHECK(q.size() == 2);
      CHECK(q.dequeue(v) && v == 3 && q.dequeue(v) && v == 5 && !q.dequeue(v) && q.isEmpty()); }

    std::cout << "--- Stack ---\n";
    { Stack<int> s; int v;
      for (int i = 1; i <= 4; ++i) s.push(i);
      CHECK(s.peek(v) && v == 4);
      CHECK(s.pop(v) && v == 4 && s.pop(v) && v == 3 && s.size() == 2);
      s.clear(); CHECK(s.isEmpty() && !s.pop(v)); }

    std::cout << "--- Heap ordering (priority, then arrival) ---\n";
    { Parcel a("A", 1, "W", "X", 1, 3), b("B", 1, "W", "X", 1, 5), c("C", 1, "W", "X", 1, 5),
             d("D", 1, "W", "X", 1, 1), e("E", 1, "W", "X", 1, 3);
      a.setArrivalOrder(1); b.setArrivalOrder(2); c.setArrivalOrder(3); d.setArrivalOrder(4); e.setArrivalOrder(5);
      UrgentHeap h; h.push(&a); h.push(&e); h.push(&d); h.push(&c); h.push(&b);
      Parcel* p; const char* expect = "BCAED"; int i = 0;
      while (h.pop(p)) CHECK(p->getTrackingID()[0] == expect[i++]);
      CHECK(i == 5);
      h.push(&a); h.push(&b); h.push(&c);
      CHECK(h.remove(&b) && !h.remove(&b) && h.size() == 2);
      CHECK(h.pop(p) && p == &c); }

    std::cout << "--- History ---\n";
    { DeliveryHistory hist;
      hist.append("T1", "x", STATUS_REGISTERED, STATUS_PENDING);
      hist.append("T2", "y", STATUS_PENDING, STATUS_DISPATCHED);
      CHECK(hist.size() == 2 && hist.removeLast() && hist.size() == 1 && hist.removeLast() && !hist.removeLast()); }

    std::cout << "--- Dispatch cycle + Undo ---\n";
    {
        Parcel n1("N1", 1, "W", "A", 1, 1), n2("N2", 1, "W", "A", 1, 1),
               u1("U1", 1, "W", "A", 1, 3), u2("U2", 1, "W", "A", 1, 5), u3("U3", 1, "W", "A", 1, 5);
        DispatchManager dm; std::string msg;
        CHECK(dm.requestNormal(&n1, msg)); CHECK(dm.requestUrgent(&u1, msg));
        CHECK(dm.requestNormal(&n2, msg)); CHECK(dm.requestUrgent(&u2, msg)); CHECK(dm.requestUrgent(&u3, msg));
        CHECK(!dm.requestNormal(&n1, msg));                       // already pending
        CHECK(dm.pendingCount() == 5 && dm.undoDepth() == 5);

        // Expected dispatch: U2 (prio5, first), U3 (prio5), U1 (prio3), N1, N2
        Parcel* first = dm.processNext();  CHECK(first == &u2 && u2.getStatus() == STATUS_DISPATCHED);
        CHECK(dm.processNext() == &u3);
        CHECK(dm.processNext() == &u1);
        CHECK(dm.processNext() == &n1);

        // undo dispatch of N1 -> back at FRONT of normal queue
        CHECK(dm.undo(msg) && n1.getStatus() == STATUS_PENDING);
        CHECK(dm.processNext() == &n1);
        CHECK(dm.processNext() == &n2);
        CHECK(dm.processNext() == nullptr);

        CHECK(dm.markDelivered(&u1, msg));
        CHECK(u1.getStatus() == STATUS_DELIVERED && dm.delivered() == 1);
        CHECK(!dm.markDelivered(&u1, msg));                       // already delivered
        CHECK(dm.undo(msg) && u1.getStatus() == STATUS_DISPATCHED && dm.delivered() == 0);

        // undo urgent dispatch puts it back into the heap
        CHECK(dm.undo(msg) && n2.getStatus() == STATUS_PENDING);
        CHECK(dm.undo(msg) && n1.getStatus() == STATUS_PENDING);
        CHECK(dm.undo(msg) && u1.getStatus() == STATUS_PENDING && dm.urgentCount() == 1);

        // undo everything down to the beginning
        while (dm.undoDepth() > 0) CHECK(dm.undo(msg));
        CHECK(!dm.undo(msg) && msg == "Nothing to undo.");
        CHECK(dm.pendingCount() == 0 && dm.historySize() == 0);
        CHECK(n1.getStatus() == STATUS_REGISTERED && u2.getStatus() == STATUS_REGISTERED);
    }

    std::cout << (failures == 0 ? "ALL MEMBER 2 TESTS PASSED" : "MEMBER 2 TESTS FAILED") << "\n";
    return failures;
}
