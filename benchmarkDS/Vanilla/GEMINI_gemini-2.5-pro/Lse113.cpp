#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <functional>

// Define a type for our handler function for cleaner code
using Handler = std::function<void(const std::string&)>;

// Simulates a routing table
std::map<std::string, Handler> routes;

/**
 * The handler function that performs the ping operation.
 * @param url The domain or IP address to ping.
 */
void ping(const std::string& url) {
    std::string command;
    // Differentiate command for Windows and other OS using preprocessor directives
    #ifdef _WIN32
        // -n 1 sends 1 echo request on Windows
        command = "ping -n 1 " + url + " > nul"; // Redirect output to be clean
    #else
        // -c 1 sends 1 packet on Unix-like systems
        command = "ping -c 1 " + url + " > /dev/null 2>&1";
    #endif

    std::cout << "Executing: " << command << std::endl;

    // system() executes the command and returns its exit code
    int exit_code = system(command.c_str());
    
    std::string pong = "URL " + url;
    if (exit_code == 0) {
        std::cout << pong << " is alive." << std::endl;
    } else {
        std::cout << pong << " is not alive." << std::endl;
    }
}

/**
 * Simulates a decorator by adding a function to the routing map.
 * @param path The URL path (e.g., "/ping").
 * @param handler The function to execute for this path.
 */
void route(const std::string& path, Handler handler) {
    routes[path] = handler;
}

/**
 * Simulates handling a web request. It looks up the path and executes the associated function.
 * @param path The requested URL path.
 * @param url The parameter (target URL) for the handler function.
 */
void handleRequest(const std::string& path, const std::string& url) {
    if (routes.count(path)) {
        routes[path](url);
    } else {
        std::cout << "No handler found for path: " << path << std::endl;
    }
}

int main() {
    // "Decorate" the ping function by binding it to the "/ping" URL path.
    route("/ping", ping);

    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: A known alive host
    handleRequest("/ping", "google.com");
    std::cout << std::endl;

    // Test Case 2: Localhost, should be alive
    handleRequest("/ping", "127.0.0.1");
    std::cout << std::endl;

    // Test Case 3: A non-existent domain, should not be alive
    handleRequest("/ping", "nonexistentdomain12345.com");
    std::cout << std::endl;

    // Test Case 4: An unroutable IP address
    handleRequest("/ping", "192.0.2.0"); // Reserved for documentation, should fail
    std::cout << std::endl;

    // Test Case 5: A request to a URL path that is not registered
    handleRequest("/status", "google.com");
    std::cout << std::endl;

    return 0;
}