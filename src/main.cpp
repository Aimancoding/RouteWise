// RouteWise - Smart Delivery and Route Optimization System (CL2001 project)
// Usage:  ./routewise [data_directory]      (default: data)
#include "RouteWiseApp.h"

int main(int argc, char* argv[]) {
    RouteWiseApp app;
    app.loadData(argc > 1 ? argv[1] : "data");
    app.run();
    return 0;
}
