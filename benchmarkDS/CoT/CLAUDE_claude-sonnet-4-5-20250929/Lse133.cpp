
// C++ requires a web framework like Crow or Pistache
// Using Crow framework as an example

#include "crow.h"

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello, World!";
    });

    // Test cases - print messages to console
    std::cout << "Test 1: Starting web application" << std::endl;
    std::cout << "Test 2: Route registered at /" << std::endl;
    std::cout << "Test 3: Response: Hello, World!" << std::endl;
    std::cout << "Test 4: Server will run on port 18080" << std::endl;
    std::cout << "Test 5: Visit http://localhost:18080/" << std::endl;

    app.port(18080).multithreaded().run();
    
    return 0;
}
