# RouteWise – Smart Delivery and Route Optimization System

Console-based C++ application (CL2001 Data Structures semester project) for a courier company:
customers and parcels, normal/urgent dispatch, delivery history with undo, fast tracking lookup,
and city road-network analysis (traversal, shortest route, minimum spanning tree).

**Every required data structure is implemented from scratch** – no `vector`, `list`, `map`,
`unordered_map`, `queue`, `stack`, `priority_queue`, etc. Only `iostream`, `fstream`, `string`
and `iomanip` are used.

## Build & run
```bash
make                 # builds ./routewise and the three test programs
./routewise          # loads sample data from ./data
./routewise mydata   # or load another folder (network.txt, customers.txt, parcels.txt)
make check           # runs the unit tests (compiled with AddressSanitizer + UBSan)
```
Without `make`: `g++ -std=c++11 -Isrc -o routewise src/main.cpp`

## Project layout
| File | Owner | Purpose |
|------|-------|---------|
| `src/Customer.h`, `src/Parcel.h` | Member 1 | Entities, `CustomerRegistry` (dynamic array) |
| `src/ParcelHashTable.h` | Member 1 | Hash table, quadratic probing, tracking-ID lookup |
| `src/ParcelBST.h`, `src/ParcelAVL.h` | Member 1 | Parcel indexes (BST and self-balancing AVL) |
| `src/TreeComparison.h` | Member 1 | BST vs AVL height comparison (menu 16) |
| `src/Queue.h`, `src/Stack.h` | Member 2 | Linked templates (also used by BFS/DFS) |
| `src/UrgentHeap.h` | Member 2 | Max-heap: priority, then arrival order |
| `src/HistoryLog.h` | Member 2 | Doubly linked delivery history |
| `src/DispatchManager.h` | Member 2 | Queue + heap + undo stack + history |
| `src/Graph.h` | Member 3 | Adjacency list + matrix, BFS, DFS, Dijkstra, Prim |
| `src/RouteWiseApp.h`, `src/main.cpp` | All | Integrated menu application |
| `src/Util.h` | All | String helpers |
| `tests/` | Each | `test_member1/2/3.cpp` |
| `data/` | All | Sample dataset |

## Where each structure is used
| Structure | Application |
|-----------|-------------|
| Dynamic memory / pointers | Customers, parcels, nodes of every list/tree/graph |
| Hash table (quadratic probing) | Parcel lookup by tracking ID; **owns** all `Parcel` objects |
| BST / AVL tree | Ordered parcel index; sorted/pre/post-order record display; height comparison |
| Queue (linked) | Normal deliveries (FIFO); BFS |
| Heap (array max-heap) | Urgent deliveries – highest priority first, ties by arrival |
| Stack (linked) | Undo of the last status-changing operation; DFS; route printing |
| Doubly linked list | Delivery history log |
| Graph (list + matrix) | City road network |
| BFS / DFS | Network exploration and reachability |
| Dijkstra (custom min-heap) | Shortest delivery route (also shown on dispatch) |
| Prim (custom min-heap) | Minimum-cost delivery network (spanning forest if disconnected) |

## Parcel life-cycle and Undo
`Registered → Pending → Dispatched → Delivered`
Menu 4/5 queue a parcel, 6 dispatches (urgent heap first, then normal queue), 7 marks it delivered.
Each step pushes an action on the undo stack and appends to the history list.
Menu 8 pops the stack and exactly reverses it (status, queue/heap membership, history entry).

## Data file formats (`|`-separated, `#` = comment)
```
network.txt   LOC|Warehouse            ROAD|Warehouse|City Center|4
customers.txt id|name|phone|address
parcels.txt   tracking|customerID|source|destination|weight_kg|priority(1-5)
```
Nothing is hard-coded – edit the files, or add data through the menu.

## Time complexity
| Operation | Complexity |
|-----------|------------|
| Hash insert / search / remove | O(1) average, O(n) worst case |
| BST insert / search / remove | O(log n) average, O(n) worst (sorted input) |
| AVL insert / search / remove | O(log n) guaranteed |
| Tree traversals | O(n) |
| Queue enqueue / dequeue | O(1) |
| Heap push / pop | O(log n); remove arbitrary parcel O(n) |
| Stack push / pop (undo) | O(1) |
| History append / remove last | O(1) |
| Graph add location / road | O(1) amortised / O(deg) |
| Road lookup (matrix) | O(1) |
| BFS, DFS | O(V + E) |
| Dijkstra | O((V + E) log V) |
| Prim MST | O(E log V) |

## Demo script (suggested for evaluation)
1. Menu 3 – search `TRK0002` (hash lookup, probe count).
2. Menu 4 ×2 and 5 ×3 – queue normal and urgent parcels; menu 9 shows dispatch order.
3. Menu 6 repeatedly – urgent parcels come first (ties by arrival), each with its shortest route.
4. Menu 7 then 8 ×2 – deliver, then undo; menu 9 / 17 confirm the restored state.
5. Menu 16 – BST vs AVL heights; menu 10 – sorted records.
6. Menu 11 → 2 – add a new location (e.g. "Seaport") with no roads; menu 12 shows it is NOT reachable.
7. Menu 12, 13, 14, 15 – BFS, DFS, shortest path (Warehouse → Airport = 18 km), MST (35 km).

## Notes for the team
* `Parcel` objects are owned by the hash table only; all other structures hold non-owning `Parcel*`.
* Menu 11 also contains *add location* and *add road*, so the 18-item menu stays intact.
