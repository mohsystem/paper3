
#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <array>
#include <memory>
#include <map>
#include <functional>

class Lse111 {
private:
    static std::map<std::string, std::function<std::string(std::string)>> routes;

public:
    // Register route (simulates decorator)
    static void registerRoute(const std::string& path, std::function<std::string(std::string)> handler) {
        routes[path] = handler;
    }
    
    // Ping function that executes system ping command
    static std::string ping(const std::string& url) {
        std::string command;
        
        #ifdef _WIN32
            command = "ping -n 4 " + url;
        #else
            command = "ping -c 4 " + url;
        #endif
        
        std::array<char, 128> buffer;
        std::string output;
        
        try {
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
            
            if (!pipe) {
                return "Error: Failed to execute ping command";
            }
            
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                output += buffer.data();
            }
            
            return output;
        } catch (const std::exception& e) {
            return std::string("Error executing ping: ") + e.what();
        }
    }
    
    static void runTests() {
        // Register the route
        registerRoute("/ping", ping);
        
        // Test cases
        std::string testUrls[] = {
            "localhost",
            "127.0.0.1",
            "google.com",
            "8.8.8.8",
            "example.com"
        };
        
        std::cout << "Testing ping functionality:\\n\\n";
        for (const auto& testUrl : testUrls) {
            std::cout << "=== Pinging: " << testUrl << " ===" << std::endl;
            std::string result = routes["/ping"](testUrl);
            std::cout << result << std::endl;
            std::cout << std::endl;
        }
    }
};

std::map<std::string, std::function<std::string(std::string)>> Lse111::routes;

int main() {
    Lse111::runTests();
    return 0;
}
