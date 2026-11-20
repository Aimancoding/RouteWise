// Member 3 tests: Graph (list + matrix), BFS, DFS, Dijkstra, Prim MST
#include <iostream>
#include <sstream>
#include <string>
#include "Graph.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::cout << "  FAIL: " #c " (line " << __LINE__ << ")\n"; ++failures; } } while (0)

static std::string orderToString(const Graph& g, int* order, int n) {
    std::string s;
    for (int i = 0; i < n; ++i) { if (i) s += ","; s += g.getName(order[i]); }
    return s;
}

int main() {
    Graph g(2);                                   // tiny capacity forces grow()
    const char* locs[] = {"Warehouse","City Center","University","Hospital","Airport",
                          "Railway Station","Industrial Area","Residential Area","Stadium"};
    for (int i = 0; i < 9; ++i) CHECK(g.addLocation(locs[i]) == i);
    CHECK(g.addLocation("warehouse") == -1);      // case-insensitive duplicate
    CHECK(g.addLocation("   ") == -1);

    struct R { const char* a; const char* b; int w; } roads[] = {
        {"Warehouse","City Center",4},{"Warehouse","Industrial Area",6},{"Warehouse","Railway Station",7},
        {"City Center","University",5},{"City Center","Hospital",3},{"City Center","Railway Station",6},
        {"University","Airport",9},{"University","Hospital",4},{"Hospital","Residential Area",5},
        {"Railway Station","Industrial Area",3},{"Railway Station","Residential Area",8},
        {"Airport","Residential Area",7},{"Stadium","University",3},{"Stadium","Residential Area",7}};
    for (size_t i = 0; i < sizeof(roads)/sizeof(roads[0]); ++i) CHECK(g.addRoad(roads[i].a, roads[i].b, roads[i].w));
    CHECK(g.getEdgeCount() == 14);
    CHECK(!g.addRoad("Warehouse", "Nowhere", 3));
    CHECK(!g.addRoad("Warehouse", "Warehouse", 3));
    CHECK(!g.addRoad("Warehouse", "Airport", 0));

    std::cout << "--- matrix is symmetric & matches list ---\n";
    CHECK(g.roadWeight(0, 1) == 4 && g.roadWeight(1, 0) == 4 && g.roadWeight(0, 4) == 0);
    g.addRoad("Warehouse", "City Center", 10);    // update existing road
    CHECK(g.getEdgeCount() == 14 && g.roadWeight(1, 0) == 10);
    g.addRoad("Warehouse", "City Center", 4);

    std::cout << "--- BFS / DFS ---\n";
    int order[16];
    int n = g.bfsOrder(0, order);
    CHECK(n == 9 && orderToString(g, order, n) ==
          "Warehouse,City Center,Industrial Area,Railway Station,University,Hospital,Residential Area,Airport,Stadium");
    n = g.dfsOrder(0, order);
    CHECK(n == 9 && orderToString(g, order, n) ==
          "Warehouse,City Center,University,Airport,Residential Area,Hospital,Railway Station,Industrial Area,Stadium");

    std::cout << "--- Dijkstra ---\n";
    { std::ostringstream os; int d = -1;
      CHECK(g.shortestPath(0, 4, os, &d) && d == 18);
      CHECK(os.str().find("Warehouse -> City Center -> University -> Airport") != std::string::npos);
      CHECK(os.str().find("Total Distance: 18 km") != std::string::npos); }
    { std::ostringstream os; int d = -1;
      CHECK(g.shortestPath(0, 0, os, &d) && d == 0); }
    { std::ostringstream os; int d = -1;
      CHECK(g.shortestPath(6, 3, os, &d) && d == 12); }      // Industrial -> Railway -> City Center -> Hospital = 3+6+3

    std::cout << "--- MST ---\n";
    { std::ostringstream os; int comps = 0;
      int total = g.minimumSpanningTree(os, &comps);
      CHECK(total == 35 && comps == 1);
      CHECK(os.str().find("Roads selected: 8") != std::string::npos); }

    std::cout << "--- Disconnected network ---\n";
    int island = g.addLocation("Seaport"), island2 = g.addLocation("Dry Port");
    g.addRoad(island, island2, 12);
    CHECK(!g.isReachable(0, island) && g.isReachable(island, island2));
    { std::ostringstream os; CHECK(!g.shortestPath(0, island, os)); CHECK(os.str().find("No route") != std::string::npos); }
    { std::ostringstream os; int comps = 0; int total = g.minimumSpanningTree(os, &comps);
      CHECK(comps == 2 && total == 35 + 12); }
    { std::ostringstream os; g.bfs(0, island, os);
      CHECK(os.str().find("NOT reachable") != std::string::npos); }

    std::cout << (failures == 0 ? "ALL MEMBER 3 TESTS PASSED" : "MEMBER 3 TESTS FAILED") << "\n";
    return failures;
}
