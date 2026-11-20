// =============================================================================
//  Parcel.h  --  Member 1
//  Parcel entity shared by ALL members.
//
//  Integration notes
//   * Member 2 (queue/heap/stack): use getPriority(), getArrivalOrder(),
//     setArrivalOrder(), getStatus()/setStatus() (save old status for undo).
//   * Member 3 (graph): getSource()/getDestination() are location names.
//   * Ownership: the ParcelHashTable owns every Parcel. Everyone else
//     (queue, heap, BST, AVL, stack, history) stores Parcel* and must NOT
//     delete them.
// =============================================================================
#ifndef ROUTEWISE_PARCEL_H
#define ROUTEWISE_PARCEL_H

#include <iostream>
#include <string>

enum DeliveryStatus {
    STATUS_REGISTERED = 0,   // created, not yet requested for delivery
    STATUS_PENDING    = 1,   // waiting in normal queue / urgent heap
    STATUS_DISPATCHED = 2,   // picked for delivery (Process Next Delivery)
    STATUS_DELIVERED  = 3    // delivered
};

inline const char* statusToString(DeliveryStatus s) {
    switch (s) {
        case STATUS_REGISTERED: return "Registered";
        case STATUS_PENDING:    return "Pending";
        case STATUS_DISPATCHED: return "Dispatched";
        case STATUS_DELIVERED:  return "Delivered";
    }
    return "Unknown";
}

class Parcel {
private:
    std::string    trackingID;
    int            customerID;
    std::string    source;
    std::string    destination;
    double         weight;        // kg
    int            priority;      // higher number = more urgent (1..5 suggested)
    DeliveryStatus status;
    long           arrivalOrder;  // used by Member 2 for FIFO tie-breaking

public:
    Parcel()
        : customerID(0), weight(0.0), priority(1),
          status(STATUS_REGISTERED), arrivalOrder(0) {}

    Parcel(const std::string& tid, int custID, const std::string& src,
           const std::string& dest, double w, int prio)
        : trackingID(tid), customerID(custID), source(src), destination(dest),
          weight(w), priority(prio), status(STATUS_REGISTERED), arrivalOrder(0) {}

    // Rule of three (explicit)
    Parcel(const Parcel& o) = default;
    Parcel& operator=(const Parcel& o) = default;
    ~Parcel() {}

    // ---- getters ----
    const std::string& getTrackingID()  const { return trackingID; }
    int                getCustomerID()  const { return customerID; }
    const std::string& getSource()      const { return source; }
    const std::string& getDestination() const { return destination; }
    double             getWeight()      const { return weight; }
    int                getPriority()    const { return priority; }
    DeliveryStatus     getStatus()      const { return status; }
    long               getArrivalOrder()const { return arrivalOrder; }

    // ---- setters ----
    void setStatus(DeliveryStatus s)    { status = s; }
    void setPriority(int p)             { priority = p; }
    void setArrivalOrder(long a)        { arrivalOrder = a; }

    void display(std::ostream& os = std::cout) const {
        os << "[" << trackingID << "] Cust#" << customerID
           << " | " << source << " -> " << destination
           << " | " << weight << " kg"
           << " | Priority " << priority
           << " | " << statusToString(status) << "\n";
    }
};

#endif
