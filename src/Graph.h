// =============================================================================
//  Graph.h  --  Member 3
//  City road network: undirected weighted graph stored TWICE
//    * Adjacency LIST   (linked edges per vertex) - used by BFS/DFS/Dijkstra/Prim
//    * Adjacency MATRIX (dynamic 2D array)        - O(1) road lookup / display
//  Weight 0 in the matrix means "no road"; road weights must be > 0 (km).
//
//  Algorithms (all custom, no STL containers):
//    BFS  - Queue<int>            O(V + E)
//    DFS  - Stack<int>            O(V + E)
//    Dijkstra - custom min-heap   O((V + E) log V)
//    Prim MST - custom min-heap   O(E log V)   (spanning forest if disconnected)
// =============================================================================
#ifndef ROUTEWISE_GRAPH_H
#define ROUTEWISE_GRAPH_H

#include <iostream>
#include <iomanip>
#include <string>
#include "Util.h"
#include "Queue.h"
#include "Stack.h"

class Graph {
private:
    struct Edge {
        int   to;
        int   weight;
        Edge* next;
        Edge(int t, int w) : to(t), weight(w), next(nullptr) {}
    };

    struct HeapItem {
        int key;      // distance (Dijkstra) or edge weight (Prim)
        int vertex;
        int from;     // predecessor (used by Prim)
    };

    // Small binary min-heap on HeapItem.key
    class MinHeap {
        HeapItem* a;
        int count, cap;
        MinHeap(const MinHeap&);
        MinHeap& operator=(const MinHeap&);
    public:
        MinHeap() : a(new HeapItem[16]), count(0), cap(16) {}
        ~MinHeap() { delete[] a; }
        bool empty() const { return count == 0; }
        void push(int key, int vertex, int from) {
            if (count == cap) {
                HeapItem* b = new HeapItem[cap * 2];
                for (int i = 0; i < count; ++i) b[i] = a[i];
                delete[] a; a = b; cap *= 2;
            }
            int i = count++;
            a[i].key = key; a[i].vertex = vertex; a[i].from = from;
            while (i > 0 && a[(i - 1) / 2].key > a[i].key) {
                HeapItem t = a[i]; a[i] = a[(i - 1) / 2]; a[(i - 1) / 2] = t;
                i = (i - 1) / 2;
            }
        }
        bool pop(HeapItem& out) {
            if (count == 0) return false;
            out = a[0];
            a[0] = a[--count];
            int i = 0;
            for (;;) {
                int l = 2 * i + 1, r = 2 * i + 2, m = i;
                if (l < count && a[l].key < a[m].key) m = l;
                if (r < count && a[r].key < a[m].key) m = r;
                if (m == i) break;
                HeapItem t = a[i]; a[i] = a[m]; a[m] = t;
                i = m;
            }
            return true;
        }
    };

    std::string* names;
    Edge**       adj;
    int**        matrix;
    int          V;
    int          cap;
    int          E;

    Graph(const Graph&);
    Graph& operator=(const Graph&);

    static const int INF = 1000000000;

    void grow() {
        int newCap = cap * 2;
        std::string* n2 = new std::string[newCap];
        Edge** a2 = new Edge*[newCap];
        int** m2 = new int*[newCap];
        for (int i = 0; i < newCap; ++i) {
            a2[i] = nullptr;
            m2[i] = new int[newCap]();           // zero-initialised
        }
        for (int i = 0; i < V; ++i) {
            n2[i] = names[i];
            a2[i] = adj[i];
            for (int j = 0; j < V; ++j) m2[i][j] = matrix[i][j];
        }
        for (int i = 0; i < cap; ++i) delete[] matrix[i];
        delete[] matrix; delete[] adj; delete[] names;
        names = n2; adj = a2; matrix = m2; cap = newCap;
    }

    void appendEdge(int from, int to, int w) {
        Edge* e = new Edge(to, w);
        if (!adj[from]) { adj[from] = e; return; }
        Edge* cur = adj[from];
        while (cur->next) cur = cur->next;
        cur->next = e;
    }

    void updateEdge(int from, int to, int w) {
        for (Edge* e = adj[from]; e; e = e->next)
            if (e->to == to) { e->weight = w; return; }
    }

public:
    explicit Graph(int initialCap = 8)
        : names(nullptr), adj(nullptr), matrix(nullptr), V(0),
          cap(initialCap < 4 ? 4 : initialCap), E(0) {
        names  = new std::string[cap];
        adj    = new Edge*[cap];
        matrix = new int*[cap];
        for (int i = 0; i < cap; ++i) { adj[i] = nullptr; matrix[i] = new int[cap](); }
    }

    ~Graph() {
        for (int i = 0; i < V; ++i) {
            Edge* e = adj[i];
            while (e) { Edge* nx = e->next; delete e; e = nx; }
        }
        for (int i = 0; i < cap; ++i) delete[] matrix[i];
        delete[] matrix; delete[] adj; delete[] names;
    }

    // ------------------------------------------------------------ building
    // Returns the vertex index, or -1 if empty name / already exists.
    int addLocation(const std::string& name) {
        std::string nm = trimStr(name);
        if (nm.empty() || findLocation(nm) != -1) return -1;
        if (V == cap) grow();
        names[V] = nm;
        adj[V]   = nullptr;
        return V++;
    }

    // Case-insensitive lookup; -1 if not found.
    int findLocation(const std::string& name) const {
        std::string key = toLowerStr(trimStr(name));
        for (int i = 0; i < V; ++i)
            if (toLowerStr(names[i]) == key) return i;
        return -1;
    }

    // Undirected road. If the road already exists its weight is updated.
    bool addRoad(int a, int b, int w) {
        if (a < 0 || b < 0 || a >= V || b >= V || a == b || w <= 0) return false;
        if (matrix[a][b] != 0) {
            updateEdge(a, b, w); updateEdge(b, a, w);
        } else {
            appendEdge(a, b, w); appendEdge(b, a, w);
            ++E;
        }
        matrix[a][b] = matrix[b][a] = w;
        return true;
    }

    bool addRoad(const std::string& from, const std::string& to, int w) {
        return addRoad(findLocation(from), findLocation(to), w);
    }

    // ---------------------------------------------------------------- info
    int getVertexCount() const { return V; }
    int getEdgeCount()   const { return E; }
    const std::string& getName(int i) const { return names[i]; }
    int roadWeight(int a, int b) const {       // matrix lookup, O(1); 0 = none
        if (a < 0 || b < 0 || a >= V || b >= V) return 0;
        return matrix[a][b];
    }

    void displayLocations(std::ostream& os = std::cout) const {
        for (int i = 0; i < V; ++i) os << "  " << i << ". " << names[i] << "\n";
    }

    void displayAdjacencyList(std::ostream& os = std::cout) const {
        os << "Adjacency List:\n";
        for (int i = 0; i < V; ++i) {
            os << "  " << names[i] << " :";
            for (Edge* e = adj[i]; e; e = e->next)
                os << " -> " << names[e->to] << " (" << e->weight << " km)";
            os << "\n";
        }
    }

    void displayAdjacencyMatrix(std::ostream& os = std::cout) const {
        os << "Adjacency Matrix (0 = no direct road, km otherwise):\n      ";
        for (int j = 0; j < V; ++j) os << std::setw(4) << j;
        os << "\n";
        for (int i = 0; i < V; ++i) {
            os << "  " << std::setw(3) << i << " ";
            for (int j = 0; j < V; ++j) os << std::setw(4) << matrix[i][j];
            os << "   " << names[i] << "\n";
        }
    }

    void displayNetwork(std::ostream& os = std::cout) const {
        os << "City network: " << V << " locations, " << E << " roads\n";
        displayAdjacencyList(os);
        os << "\n";
        displayAdjacencyMatrix(os);
    }

    // ------------------------------------------------------------ traversals
    // Fills order[] (size >= V) with the BFS visit order; returns #visited.
    int bfsOrder(int start, int* order) const {
        if (start < 0 || start >= V) return 0;
        bool* visited = new bool[V]();
        Queue<int> q;
        int n = 0, v;
        visited[start] = true;
        q.enqueue(start);
        while (q.dequeue(v)) {
            order[n++] = v;
            for (Edge* e = adj[v]; e; e = e->next)
                if (!visited[e->to]) { visited[e->to] = true; q.enqueue(e->to); }
        }
        delete[] visited;
        return n;
    }

    // Iterative DFS with an explicit Stack; same order as recursive DFS.
    int dfsOrder(int start, int* order) const {
        if (start < 0 || start >= V) return 0;
        bool* visited = new bool[V]();
        Stack<int> st;
        int n = 0, v;
        st.push(start);
        while (st.pop(v)) {
            if (visited[v]) continue;
            visited[v] = true;
            order[n++] = v;
            int deg = 0;
            for (Edge* e = adj[v]; e; e = e->next) ++deg;
            int* nb = new int[deg > 0 ? deg : 1];
            int k = 0;
            for (Edge* e = adj[v]; e; e = e->next) nb[k++] = e->to;
            for (int i = k - 1; i >= 0; --i)         // reversed so first neighbour pops first
                if (!visited[nb[i]]) st.push(nb[i]);
            delete[] nb;
        }
        delete[] visited;
        return n;
    }

    bool isReachable(int src, int dst) const {
        if (src < 0 || dst < 0 || src >= V || dst >= V) return false;
        int* order = new int[V];
        int n = bfsOrder(src, order);
        bool found = false;
        for (int i = 0; i < n; ++i) if (order[i] == dst) { found = true; break; }
        delete[] order;
        return found;
    }

    // Prints the traversal. If dest >= 0 also reports reachability.
    void bfs(int start, int dest, std::ostream& os = std::cout) const {
        report("BFS", start, dest, true, os);
    }
    void dfs(int start, int dest, std::ostream& os = std::cout) const {
        report("DFS", start, dest, false, os);
    }

    // ------------------------------------------------------------- Dijkstra
    // Prints the route and distance. Returns true if a route exists;
    // outDist (optional) receives the total distance.
    bool shortestPath(int src, int dst, std::ostream& os = std::cout, int* outDist = nullptr) const {
        if (src < 0 || dst < 0 || src >= V || dst >= V) return false;

        int*  dist = new int[V];
        int*  prev = new int[V];
        bool* done = new bool[V]();
        for (int i = 0; i < V; ++i) { dist[i] = INF; prev[i] = -1; }

        MinHeap pq;
        dist[src] = 0;
        pq.push(0, src, -1);
        HeapItem it;
        while (pq.pop(it)) {
            int u = it.vertex;
            if (done[u]) continue;                 // stale entry
            done[u] = true;
            if (u == dst) break;
            for (Edge* e = adj[u]; e; e = e->next) {
                int nd = dist[u] + e->weight;
                if (nd < dist[e->to]) {
                    dist[e->to] = nd;
                    prev[e->to] = u;
                    pq.push(nd, e->to, u);
                }
            }
        }

        bool ok = dist[dst] != INF;
        os << "Source: " << names[src] << "\nDestination: " << names[dst] << "\n";
        if (!ok) {
            os << "No route exists between these locations.\n";
        } else {
            Stack<int> path;                       // reverse the predecessor chain
            for (int v = dst; v != -1; v = prev[v]) path.push(v);
            os << "Recommended Route:\n";
            int v; bool first = true;
            while (path.pop(v)) {
                if (!first) os << " -> ";
                os << names[v];
                first = false;
            }
            os << "\nTotal Distance: " << dist[dst] << " km\n";
            if (outDist) *outDist = dist[dst];
        }
        delete[] dist; delete[] prev; delete[] done;
        return ok;
    }

    // ------------------------------------------------------- Prim's MST
    // Returns total cost; prints chosen roads. For a disconnected network a
    // minimum spanning FOREST is produced (one tree per component).
    int minimumSpanningTree(std::ostream& os = std::cout, int* outComponents = nullptr) const {
        if (V == 0) { os << "The network is empty.\n"; return 0; }
        bool* inTree = new bool[V]();
        int total = 0, edgesUsed = 0, components = 0;

        os << "Selected connections:\n";
        for (int s = 0; s < V; ++s) {
            if (inTree[s]) continue;
            ++components;
            MinHeap pq;
            pq.push(0, s, -1);
            HeapItem it;
            while (pq.pop(it)) {
                int u = it.vertex;
                if (inTree[u]) continue;
                inTree[u] = true;
                if (it.from != -1) {
                    os << "  " << names[it.from] << " -- " << names[u]
                       << " (" << it.key << " km)\n";
                    total += it.key;
                    ++edgesUsed;
                }
                for (Edge* e = adj[u]; e; e = e->next)
                    if (!inTree[e->to]) pq.push(e->weight, e->to, u);
            }
        }
        if (components > 1)
            os << "Note: network is disconnected - showing a minimum spanning forest ("
               << components << " components).\n";
        os << "Roads selected: " << edgesUsed << "\nTotal Network Cost: " << total << " km\n";
        if (outComponents) *outComponents = components;
        delete[] inTree;
        return total;
    }

private:
    void report(const char* title, int start, int dest, bool isBfs, std::ostream& os) const {
        if (start < 0 || start >= V) { os << "Invalid start location.\n"; return; }
        int* order = new int[V];
        int n = isBfs ? bfsOrder(start, order) : dfsOrder(start, order);
        os << title << " from " << names[start] << ":\n  ";
        for (int i = 0; i < n; ++i) {
            if (i) os << " -> ";
            os << names[order[i]];
        }
        os << "\n  Locations reached: " << n << " of " << V << "\n";
        if (dest >= 0 && dest < V) {
            bool found = false;
            for (int i = 0; i < n; ++i) if (order[i] == dest) { found = true; break; }
            os << "  " << names[dest] << " is " << (found ? "REACHABLE" : "NOT reachable")
               << " from " << names[start] << ".\n";
        }
        delete[] order;
    }
};

#endif
