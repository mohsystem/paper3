
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <array>
#include <memory>
#include <cstdio>
#include <sstream>

class Lse112 {
private:
    static std::map<std::string, std::function<std::string(std::string)>> routes;
    
public:
    // Register route (decorator pattern)
    static void route(const std::string& path, std::function<std::string(std::string)> handler) {
        routes[path] = handler;
    }
    
    // Ping function that executes system command
    static std::string ping(const std::string& url) {
        try {
            std::string command;
            
            #ifdef _WIN32
                command = "ping -n 4 " + url;
            #else
                command = "ping -c 4 " + url;
            #endif
            
            std::array<char, 128> buffer;
            std::string result;
            
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
            
            if (!pipe) {
                return "Error: Failed to execute ping command";
            }
            
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            
            return result;
            
        } catch (const std::exception& e) {
            return "Error executing ping: " + std::string(e.what());
        }
    }
    
    // Simulate request handling
    static std::string handleRequest(const std::string& path, const std::string& param) {
        auto it = routes.find(path);
        if (it != routes.end()) {
            return it->second(param);
        }
        return "404 Not Found";
    }
};

// Initialize static member
std::map<std::string, std::function<std::string(std::string)>> Lse112::routes;

int main() {
    // Register the ping route (decorator pattern)
    Lse112::route("/ping", [](const std::string& url) {
        return Lse112::ping(url);
    });
    
    // Test cases
    std::cout << "Test Case 1: Ping localhost" << std::endl;
    std::cout << Lse112::handleRequest("/ping", "localhost") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 2: Ping 127.0.0.1" << std::endl;
    std::cout << Lse112::handleRequest("/ping", "127.0.0.1") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 3: Ping google.com" << std::endl;
    std::cout << Lse112::handleRequest("/ping", "google.com") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 4: Ping invalid URL" << std::endl;
    std::cout << Lse112::handleRequest("/ping", "invalid-url-12345.com") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 5: Invalid route" << std::endl;
    std::cout << Lse112::handleRequest("/invalid", "test") << std::endl;
    
    return 0;
}
