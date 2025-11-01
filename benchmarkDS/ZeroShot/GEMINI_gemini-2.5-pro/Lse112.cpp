/*
 * Instructions:
 * 1. This code requires the 'cpp-httplib' single-header library.
 *    Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 *    and place it in the same directory as this source file.
 * 2. Compile with a C++11 compliant compiler.
 *    Example on Linux/macOS: g++ -std=c++11 -o server Lse112.cpp -pthread
 *    Example on Windows (MinGW): g++ -std=c++11 -o server.exe Lse112.cpp -lws2_32 -lwsock32
 * 3. Run the compiled executable: ./server
 */
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

#define CPPHTTPLIB_OPENSSL_SUPPORT // Comment out if you don't need SSL
#include "httplib.h" // Must be in the same directory or in include path

// The URL/IP to ping is a hardcoded, trusted constant to prevent command injection.
const std::string URL_TO_PING = "8.8.8.8";

/**
 * Executes a system command and returns its output.
 * @param cmd The command to execute.
 * @return The standard output of the command.
 */
std::string exec_command(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    // Use popen to execute the command and open a pipe to read its output.
    // This is safer than system() when you need to capture output. Since the
    // command string is built from a hardcoded constant, it is secure.
#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
#endif
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


int main(void) {
    /*
     * Test Cases:
     * 1. Compile and run the server as per the instructions above.
     * 2. Open a terminal or command prompt.
     * 3. Run the following command: curl http://localhost:8081/ping
     *    - This should return the output of pinging 8.8.8.8.
     * 4. Access http://localhost:8081/ in a browser, which should show 404 Not Found.
     * 5. The design is secure because the address to be pinged is hardcoded.
     *    Any attempt to pass a parameter in the URL (e.g., /ping?url=;ls)
     *    will be ignored by the server logic.
    */
    httplib::Server svr;

    // Binds the lambda function to the "/ping" URL for GET requests.
    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        std::string command;
        
        #ifdef _WIN32
            // For Windows: ping -n 4 <url>
            command = "ping -n 4 " + URL_TO_PING;
        #else
            // For Linux/macOS: ping -c 4 <url>
            command = "ping -c 4 " + URL_TO_PING;
        #endif

        try {
            std::string output = exec_command(command.c_str());
            res.set_content(output, "text/plain");
            res.status = 200;
        } catch (const std::exception& e) {
            res.set_content("Error executing command: " + std::string(e.what()), "text/plain");
            res.status = 500;
        }
    });
    
    int port = 8081;
    std::cout << "Server started on port: " << port << std::endl;
    std::cout << "Access http://localhost:8081/ping to execute the ping command." << std::endl;
    svr.listen("0.0.0.0", port);

    return 0;
}