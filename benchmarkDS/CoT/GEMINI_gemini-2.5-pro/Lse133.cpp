// To compile and run this code, you need the 'cpp-httplib' library.
// It's a header-only library. Download 'httplib.h' from its GitHub repository.
// Place 'httplib.h' in the same directory or in your include path.
//
// Compile with g++:
// g++ -o server Lse133.cpp -pthread -std=c++11
//
// Then run the executable:
// ./server

#include <iostream>
#include "httplib.h" // 1. The first line imports the web application class/library.

// A class to encapsulate the server logic, similar to the Java example.
class Lse133 {
public:
    void run() {
        // 2. The second line creates the application object.
        httplib::Server svr;

        // 3. The third line is a decorator-like route definition. It tells the application
        // which URL ("/") should call the associated lambda function.
        // 4. The fourth line is the lambda function that is called.
        svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
            // 5. The fifth line returns/sets the message to the user.
            res.set_content("Hello from C++ Web Server!", "text/plain");
        });

        std::cout << "Server started on port 8080. Open http://localhost:8080/ in a browser." << std::endl;
        
        // 6. The sixth line runs the application (listens for connections).
        svr.listen("0.0.0.0", 8080);
    }
};

int main() {
    Lse133 webApp;
    webApp.run();

    // Test Cases for a web server are typically done by making HTTP requests.
    // After running this main method, you can perform these 5 manual tests:
    // 1. Open a web browser and navigate to http://localhost:8080/
    // 2. Use curl in a terminal: curl http://localhost:8080/
    // 3. Test a different, non-existent path to see a 404 error: curl http://localhost:8080/test
    // 4. Use a tool like Postman to send a GET request to http://localhost:8080/
    // 5. Check server logs/console output for confirmation message.
    
    return 0;
}