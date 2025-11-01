/*
 * This code requires the cpp-httplib library.
 * It's a header-only library, so you just need to download httplib.h and place it
 * in the same directory or in your include path.
 * You can get it from: https://github.com/yhirose/cpp-httplib
 *
 * Compilation command:
 * g++ -std=c++17 -o server Lse112.cpp -lpthread
 * (on Windows, you may need to link against Ws2_32.lib: 
 *  g++ -std=c++17 -o server.exe Lse112.cpp -lpthread -lWs2_32)
 */

#define CPPHTTPLIB_OPENSSL_SUPPORT // Comment out if you don't have OpenSSL headers
#include "httplib.h" // Assuming httplib.h is in the include path
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sstream>

// The URL to ping is stored as a variable.
// This must be a static, trusted value to prevent command injection.
const std::string PING_TARGET_URL = "8.8.8.8";

/**
 * Executes a system command and returns its output.
 * @param cmd The command to execute. Should be constructed from trusted strings.
 * @return The stdout and stderr of the command.
 */
std::string exec(const char* cmd) {
    // Using popen is simpler but it invokes a shell.
    // Since the command is constructed from hardcoded strings, the risk is minimal.
    // In a high-security context, fork/exec would be preferred to avoid the shell.
    #ifdef _WIN32
        FILE* pipe = _popen(cmd, "r");
    #else
        FILE* pipe = popen(cmd, "r");
    #endif

    if (!pipe) {
        return "popen() failed!";
    }

    std::array<char, 128> buffer;
    std::stringstream result;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result << buffer.data();
    }

    #ifdef _WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif
    return result.str();
}

/**
 * Constructs and runs the ping command for the globally defined URL.
 * @return The output of the ping command.
 */
std::string runPingCommand() {
    std::string command;
    #ifdef _WIN32
        command = "ping -n 4 " + PING_TARGET_URL;
    #else
        command = "ping -c 4 " + PING_TARGET_URL;
    #endif
    
    // Validate that the URL does not contain malicious characters.
    // This is belt-and-suspenders as it's a constant, but demonstrates good practice.
    if (PING_TARGET_URL.find_first_not_of("0123456789.") != std::string::npos) {
         return "Invalid characters in target URL.";
    }

    return exec(command.c_str());
}

int main(void) {
    httplib::Server svr;

    // The Get method binds the "/ping" URL to the provided lambda function.
    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        std::string ping_output = runPingCommand();
        res.set_content(ping_output, "text/plain");
    });

    int port = 8080;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Access http://localhost:" << port << "/ping to execute the command." << std::endl;
    
    // --- Test Cases ---
    // To test the server, compile and run this program, then use a tool like curl or a web browser.
    // Test Case 1: Access the ping URL from a browser:
    // Open http://localhost:8080/ping
    //
    // Test Case 2: Use curl in a terminal:
    // curl http://localhost:8080/ping
    //
    // Test Case 3: Test non-existent URL (should give 404 Not Found):
    // curl http://localhost:8080/other
    //
    // Test Case 4: Test a different HTTP method (should give 405 Method Not Allowed):
    // curl -X POST http://localhost:8080/ping
    //
    // Test Case 5: The main function itself acts as the primary test harness by running the server.
    // The server will run until the program is manually stopped.

    svr.listen("0.0.0.0", port);

    return 0;
}