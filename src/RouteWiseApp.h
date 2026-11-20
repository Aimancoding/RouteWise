// =============================================================================
//  RouteWiseApp.h  --  Integration (all members)
//  The console application that wires every module together:
//    Member 1: CustomerRegistry, ParcelHashTable, ParcelBST, ParcelAVL
//    Member 2: DispatchManager (Queue, UrgentHeap, Stack, DeliveryHistory)
//    Member 3: Graph (adjacency list + matrix, BFS, DFS, Dijkstra, Prim)
// =============================================================================
#ifndef ROUTEWISE_APP_H
#define ROUTEWISE_APP_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "Util.h"
#include "Customer.h"
#include "Parcel.h"
#include "ParcelHashTable.h"
#include "ParcelBST.h"
#include "ParcelAVL.h"
#include "TreeComparison.h"
#include "DispatchManager.h"
#include "Graph.h"

class RouteWiseApp {
private:
    CustomerRegistry customers;
    ParcelHashTable  parcels;     // OWNS all Parcel objects
    ParcelBST        bstIndex;    // non-owning index
    ParcelAVL        avlIndex;    // non-owning index
    DispatchManager  dispatch;
    Graph            city;
    int              nextTrackingNo;

    // ------------------------------------------------------------ input
    static std::string ask(const std::string& prompt) {
        std::cout << prompt;
        std::string s;
        if (!std::getline(std::cin, s)) {
            std::cout << "\nInput closed. Exiting.\n";
            std::exit(0);
        }
        return trimStr(s);
    }

    static int askInt(const std::string& prompt, int lo, int hi) {
        for (;;) {
            std::string s = ask(prompt);
            bool ok = !s.empty();
            size_t i = (ok && s[0] == '-') ? 1 : 0;
            if (i == s.length()) ok = false;
            for (; ok && i < s.length(); ++i) if (!std::isdigit((unsigned char)s[i])) ok = false;
            if (ok && s.length() < 10) {
                int v = std::atoi(s.c_str());
                if (v >= lo && v <= hi) return v;
            }
            std::cout << "  Please enter a whole number between " << lo << " and " << hi << ".\n";
        }
    }

    static double askPositiveDouble(const std::string& prompt) {
        for (;;) {
            std::string s = ask(prompt);
            char* end = nullptr;
            double v = std::strtod(s.c_str(), &end);
            if (!s.empty() && end && *end == '\0' && v > 0) return v;
            std::cout << "  Please enter a number greater than 0.\n";
        }
    }

    static std::string askNonEmpty(const std::string& prompt) {
        for (;;) {
            std::string s = ask(prompt);
            if (!s.empty()) return s;
            std::cout << "  This field cannot be empty.\n";
        }
    }

    // Asks for a location until it exists in the network. Blank -> defaultIdx.
    int askLocation(const std::string& prompt, int defaultIdx = -1) {
        for (;;) {
            std::string s = ask(prompt);
            if (s.empty() && defaultIdx >= 0) return defaultIdx;
            int idx = city.findLocation(s);
            if (idx >= 0) return idx;
            std::cout << "  Unknown location. Available locations:\n";
            city.displayLocations();
        }
    }

    // ----------------------------------------------------------- helpers
    std::string newTrackingID() {
        for (;;) {
            std::string n = std::to_string(nextTrackingNo++);
            while (n.length() < 4) n = "0" + n;
            std::string id = "TRK" + n;
            if (!parcels.search(id)) return id;
        }
    }

    // Single place where a parcel enters the system: hash table (owner) + BST + AVL.
    bool storeParcel(Parcel* p, std::string& err) {
        if (!customers.findCustomer(p->getCustomerID())) {
            err = "Customer #" + std::to_string(p->getCustomerID()) + " is not registered.";
            delete p; return false;
        }
        if (city.findLocation(p->getSource()) < 0 || city.findLocation(p->getDestination()) < 0) {
            err = "Source/destination is not a location in the city network.";
            delete p; return false;
        }
        if (p->getPriority() < 1 || p->getPriority() > 5 || p->getWeight() <= 0) {
            err = "Invalid priority (1-5) or weight (> 0).";
            delete p; return false;
        }
        if (!parcels.insert(p)) {            // duplicate -> table did not take ownership
            err = "Tracking ID " + p->getTrackingID() + " already exists.";
            delete p; return false;
        }
        bstIndex.insert(p);
        avlIndex.insert(p);
        return true;
    }

    Parcel* askParcel(const std::string& prompt) {
        std::string id = ask(prompt);
        Parcel* p = parcels.search(id);
        if (!p) std::cout << "  No parcel with tracking ID '" << id << "'.\n";
        return p;
    }

    // ------------------------------------------------------- file loading
    int loadCustomers(const std::string& path) {
        std::ifstream in(path.c_str());
        if (!in) { std::cout << "  (could not open " << path << ")\n"; return -1; }
        std::string line, f[4]; int n = 0;
        while (std::getline(in, line)) {
            line = trimStr(line);
            if (line.empty() || line[0] == '#') continue;
            if (splitFields(line, f, 4) != 4) continue;
            if (customers.addCustomer(std::atoi(f[0].c_str()), f[1], f[2], f[3])) ++n;
        }
        return n;
    }

    int loadNetwork(const std::string& path) {
        std::ifstream in(path.c_str());
        if (!in) { std::cout << "  (could not open " << path << ")\n"; return -1; }
        std::string line, f[4]; int n = 0;
        while (std::getline(in, line)) {
            line = trimStr(line);
            if (line.empty() || line[0] == '#') continue;
            int k = splitFields(line, f, 4);
            if (k >= 2 && toLowerStr(f[0]) == "loc") {
                if (city.addLocation(f[1]) >= 0) ++n;
            } else if (k == 4 && toLowerStr(f[0]) == "road") {
                city.addLocation(f[1]); city.addLocation(f[2]);   // auto-create if missing
                if (city.addRoad(f[1], f[2], std::atoi(f[3].c_str()))) ++n;
            }
        }
        return n;
    }

    int loadParcels(const std::string& path) {
        std::ifstream in(path.c_str());
        if (!in) { std::cout << "  (could not open " << path << ")\n"; return -1; }
        std::string line, f[6], err; int n = 0;
        while (std::getline(in, line)) {
            line = trimStr(line);
            if (line.empty() || line[0] == '#') continue;
            if (splitFields(line, f, 6) != 6) continue;
            Parcel* p = new Parcel(f[0], std::atoi(f[1].c_str()), f[2], f[3],
                                   std::atof(f[4].c_str()), std::atoi(f[5].c_str()));
            if (storeParcel(p, err)) ++n;
            else std::cout << "  skipped " << f[0] << ": " << err << "\n";
        }
        return n;
    }

    // ----------------------------------------------------------- menu actions
    void registerCustomer() {
        int id = askInt("Customer ID: ", 1, 999999999);
        if (customers.findCustomer(id)) { std::cout << "  That customer ID already exists.\n"; return; }
        std::string name  = askNonEmpty("Name: ");
        std::string phone = askNonEmpty("Phone: ");
        std::string addr  = askNonEmpty("Address: ");
        customers.addCustomer(id, name, phone, addr);
        std::cout << "  Customer registered.\n";
    }

    void registerParcel() {
        if (city.getVertexCount() < 2) { std::cout << "  Build the city network first (option 11).\n"; return; }
        int cid = askInt("Customer ID: ", 1, 999999999);
        if (!customers.findCustomer(cid)) { std::cout << "  Customer not registered. Use option 1 first.\n"; return; }
        std::string tid = ask("Tracking ID (leave blank to auto-generate): ");
        if (tid.empty()) tid = newTrackingID();
        std::cout << "Locations:\n"; city.displayLocations();
        int s = askLocation("Source (blank = Warehouse): ", city.findLocation("Warehouse"));
        int d = askLocation("Destination: ");
        double w = askPositiveDouble("Weight (kg): ");
        int pr = askInt("Priority (1 = lowest ... 5 = highest): ", 1, 5);

        std::string err;
        Parcel* p = new Parcel(tid, cid, city.getName(s), city.getName(d), w, pr);
        if (storeParcel(p, err)) std::cout << "  Parcel registered with tracking ID " << tid << ".\n";
        else std::cout << "  Failed: " << err << "\n";
    }

    void searchParcel() {
        std::string id = ask("Tracking ID: ");
        Parcel* p = parcels.search(id);
        int probes = parcels.getLastProbes();
        if (!p) { std::cout << "  Not found (" << probes << " probe(s) in hash table).\n"; return; }
        std::cout << "  Found via hash table in " << probes << " probe(s):\n  ";
        p->display();
        Customer* c = customers.findCustomer(p->getCustomerID());
        if (c) { std::cout << "  Owner: "; c->display(); }
    }

    void addRequest(bool urgent) {
        Parcel* p = askParcel("Tracking ID: ");
        if (!p) return;
        std::string msg;
        bool ok = urgent ? dispatch.requestUrgent(p, msg) : dispatch.requestNormal(p, msg);
        std::cout << "  " << msg << "\n";
        (void)ok;
    }

    void processNext() {
        Parcel* p = dispatch.processNext();
        if (!p) { std::cout << "  No pending deliveries.\n"; return; }
        std::cout << "  Dispatching: "; p->display();
        int s = city.findLocation(p->getSource()), d = city.findLocation(p->getDestination());
        if (s >= 0 && d >= 0) { std::cout << "\n"; city.shortestPath(s, d); }
    }

    void markDelivered() {
        Parcel* p = askParcel("Tracking ID: ");
        if (!p) return;
        std::string msg;
        dispatch.markDelivered(p, msg);
        std::cout << "  " << msg << "\n";
    }

    void undoLast() {
        std::string msg;
        dispatch.undo(msg);
        std::cout << "  " << msg << "\n";
    }

    void displayRecords() {
        if (parcels.size() == 0) { std::cout << "  No parcel records.\n"; return; }
        std::cout << "1. Hash table order\n2. Sorted (AVL in-order)\n3. AVL pre-order\n4. AVL post-order\n";
        int c = askInt("View: ", 1, 4);
        if (c == 1) parcels.displayAll();
        else if (c == 2) avlIndex.inorder();
        else if (c == 3) avlIndex.preorder();
        else avlIndex.postorder();
    }

    void networkMenu() {
        std::cout << "1. Display network\n2. Add location\n3. Add road\n";
        int c = askInt("Choice: ", 1, 3);
        if (c == 1) city.displayNetwork();
        else if (c == 2) {
            std::string n = askNonEmpty("New location name: ");
            std::cout << (city.addLocation(n) >= 0 ? "  Location added.\n" : "  Location already exists.\n");
        } else {
            if (city.getVertexCount() < 2) { std::cout << "  Need at least two locations.\n"; return; }
            city.displayLocations();
            int a = askLocation("From: "), b = askLocation("To: ");
            if (a == b) { std::cout << "  A road needs two different locations.\n"; return; }
            int w = askInt("Distance (km): ", 1, 1000000);
            city.addRoad(a, b, w);
            std::cout << "  Road saved.\n";
        }
    }

    void traversal(bool isBfs) {
        if (city.getVertexCount() == 0) { std::cout << "  The network is empty.\n"; return; }
        city.displayLocations();
        int wh = city.findLocation("Warehouse");
        int s = askLocation("Start location (blank = Warehouse): ", wh >= 0 ? wh : 0);
        std::string t = ask("Destination to check reachability (blank = skip): ");
        int d = t.empty() ? -1 : city.findLocation(t);
        if (!t.empty() && d < 0) std::cout << "  (unknown destination, skipping reachability check)\n";
        if (isBfs) city.bfs(s, d); else city.dfs(s, d);
    }

    void shortestRoute() {
        if (city.getVertexCount() < 2) { std::cout << "  Need at least two locations.\n"; return; }
        city.displayLocations();
        int s = askLocation("Source: "), d = askLocation("Destination: ");
        city.shortestPath(s, d);
    }

    void statistics() {
        std::cout << "======== System Statistics ========\n"
                  << "Customers registered : " << customers.size() << "\n"
                  << "Parcels registered   : " << parcels.size() << "\n"
                  << "Hash table           : capacity " << parcels.getCapacity()
                  << ", load factor " << parcels.loadFactor()
                  << ", avg probes " << parcels.averageProbes() << "\n"
                  << "BST height           : " << bstIndex.height() << "\n"
                  << "AVL height           : " << avlIndex.height()
                  << " (rotations: " << avlIndex.getRotationCount() << ")\n"
                  << "Pending (normal)     : " << dispatch.normalCount() << "\n"
                  << "Pending (urgent)     : " << dispatch.urgentCount() << "\n"
                  << "Delivered            : " << dispatch.delivered() << "\n"
                  << "History entries      : " << dispatch.historySize() << "\n"
                  << "Undo stack depth     : " << dispatch.undoDepth() << "\n"
                  << "Network              : " << city.getVertexCount() << " locations, "
                  << city.getEdgeCount() << " roads\n";
        if (dispatch.historySize() > 0) { std::cout << "--- Delivery history ---\n"; dispatch.displayHistory(); }
    }

    void printMenu() const {
        std::cout <<
            "\n=========================================\n"
            "        ROUTEWISE DELIVERY SYSTEM\n"
            "=========================================\n"
            "1.  Register Customer\n"
            "2.  Register Parcel\n"
            "3.  Search Parcel\n"
            "4.  Add Normal Delivery Request\n"
            "5.  Add Urgent Delivery Request\n"
            "6.  Process Next Delivery\n"
            "7.  Mark Parcel as Delivered\n"
            "8.  Undo Last Operation\n"
            "9.  Display Pending Deliveries\n"
            "10. Display Parcel Records\n"
            "11. Display City Network (also add locations/roads)\n"
            "12. BFS Network Traversal\n"
            "13. DFS Network Traversal\n"
            "14. Find Shortest Delivery Route\n"
            "15. Generate Minimum Delivery Network\n"
            "16. Compare BST and AVL Performance\n"
            "17. Display System Statistics\n"
            "18. Exit\n";
    }

public:
    RouteWiseApp() : parcels(31), nextTrackingNo(1) {}

    void loadData(const std::string& dir) {
        std::cout << "Loading data from '" << dir << "' ...\n";
        int n = loadNetwork(dir + "/network.txt");
        if (n >= 0) std::cout << "  network : " << city.getVertexCount() << " locations, "
                              << city.getEdgeCount() << " roads\n";
        n = loadCustomers(dir + "/customers.txt");
        if (n >= 0) std::cout << "  customers: " << n << "\n";
        n = loadParcels(dir + "/parcels.txt");
        if (n >= 0) std::cout << "  parcels  : " << n << "\n";
    }

    void run() {
        int choice;
        do {
            printMenu();
            choice = askInt("Enter choice: ", 1, 18);
            std::cout << "\n";
            switch (choice) {
                case 1:  registerCustomer();      break;
                case 2:  registerParcel();        break;
                case 3:  searchParcel();          break;
                case 4:  addRequest(false);       break;
                case 5:  addRequest(true);        break;
                case 6:  processNext();           break;
                case 7:  markDelivered();         break;
                case 8:  undoLast();              break;
                case 9:  dispatch.displayPending(); break;
                case 10: displayRecords();        break;
                case 11: networkMenu();           break;
                case 12: traversal(true);         break;
                case 13: traversal(false);        break;
                case 14: shortestRoute();         break;
                case 15: city.minimumSpanningTree(); break;
                case 16: compareBSTandAVL(parcels); break;
                case 17: statistics();            break;
                case 18: std::cout << "Goodbye!\n"; break;
            }
        } while (choice != 18);
    }
};

#endif
