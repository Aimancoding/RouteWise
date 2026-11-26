// =============================================================================
//  Customer.h  --  Member 1
//  Customer entity + CustomerRegistry (dynamic array of Customer*).
//  No STL containers are used. Memory is released in destructors.
// =============================================================================
#ifndef ROUTEWISE_CUSTOMER_H
#define ROUTEWISE_CUSTOMER_H

#include <iostream>
#include <string>

class Customer {
private:
    int         customerID;
    std::string name;
    std::string phone;
    std::string address;

public:
    Customer() : customerID(0) {}
    Customer(int id, const std::string& n, const std::string& ph, const std::string& addr)
        : customerID(id), name(n), phone(ph), address(addr) {}

    // Rule of three (all members are value types, so copying is safe)
    Customer(const Customer& o) = default;
    Customer& operator=(const Customer& o) = default;
    ~Customer() {}

    int                getID()      const { return customerID; }
    const std::string& getName()    const { return name; }
    const std::string& getPhone()   const { return phone; }
    const std::string& getAddress() const { return address; }

    void setName(const std::string& n)    { name = n; }
    void setPhone(const std::string& p)   { phone = p; }
    void setAddress(const std::string& a) { address = a; }

    void display(std::ostream& os = std::cout) const {
        os << "Customer #" << customerID << " | " << name
           << " | " << phone << " | " << address << "\n";
    }
};

// -----------------------------------------------------------------------------
//  CustomerRegistry: owns every Customer it stores (grows dynamically).
// -----------------------------------------------------------------------------
class CustomerRegistry {
private:
    Customer** data;
    int        count;
    int        capacity;

    void grow() {
        int newCap = capacity * 2;
        Customer** bigger = new Customer*[newCap];
        for (int i = 0; i < count; ++i) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCap;
    }

    // Not copyable: it owns raw pointers
    CustomerRegistry(const CustomerRegistry&);
    CustomerRegistry& operator=(const CustomerRegistry&);

public:
    CustomerRegistry() : data(new Customer*[8]), count(0), capacity(8) {}

    ~CustomerRegistry() {
        for (int i = 0; i < count; ++i) delete data[i];
        delete[] data;
    }

    // Returns false if the ID already exists.
    bool addCustomer(int id, const std::string& name,
                     const std::string& phone, const std::string& address) {
        if (findCustomer(id)) return false;
        if (count == capacity) grow();
        data[count++] = new Customer(id, name, phone, address);
        return true;
    }

    Customer* findCustomer(int id) const {
        for (int i = 0; i < count; ++i)
            if (data[i]->getID() == id) return data[i];
        return nullptr;
    }

    int size() const { return count; }

    // i-th customer (0 .. size()-1), or nullptr. Used when saving to file.
    Customer* getAt(int i) const { return (i >= 0 && i < count) ? data[i] : nullptr; }

    void displayAll(std::ostream& os = std::cout) const {
        if (count == 0) { os << "No customers registered.\n"; return; }
        for (int i = 0; i < count; ++i) data[i]->display(os);
    }
};

#endif
