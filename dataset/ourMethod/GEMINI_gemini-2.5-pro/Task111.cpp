#include <iostream>
#include <string>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <csignal>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <xmlrpc-c/client_simple.hpp>

// Compilation command:
// g++ -std=c++11 task.cpp -o task_server -lxmlrpc_server_abyss++ -lxmlrpc_server++ -lxmlrpc_client++ -lxmlrpc++ -lxmlrpc_xmlparse -lxmlrpc_xmltok -lxmlrpc_util -pthread

// This method handles the "sample.add" XML-RPC call
class sample_add_method : public xmlrpc_c::method {
public:
    sample_add_method() {
        this->_signature = "i:ii"; // Returns integer, takes two integers
        this->_help = "This method adds two integers.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value * const  retvalP) {
        
        // Rule #3: Validate input count and types.
        paramList.verifyEnd(2);
        int const addend(paramList.getInt(0));
        int const adder(paramList.getInt(1));

        // Rule #8: Use exceptions for error handling.
        // xmlrpc-c will catch this and return a fault.
        *retvalP = xmlrpc_c::value_int(addend + adder);
    }
};

// This method handles the "sample.divide" XML-RPC call
class sample_divide_method : public xmlrpc_c::method {
public:
    sample_divide_method() {
        this->_signature = "d:dd"; // Returns double, takes two doubles
        this->_help = "This method divides two numbers.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value * const  retvalP) {

        paramList.verifyEnd(2);
        double const dividend(paramList.getDouble(0));
        double const divisor(paramList.getDouble(1));

        // Rule #3: Validate input value.
        if (divisor == 0) {
            // Rule #8: Throwing an xmlrpc_c::fault creates a proper XML-RPC fault response.
            throw xmlrpc_c::fault("Division by zero is not allowed.",
                                  xmlrpc_c::fault::CODE_UNSPECIFIED);
        }

        *retvalP = xmlrpc_c::value_double(dividend / divisor);
    }
};

// Global pointer to the server to allow shutdown from a different thread/handler
static xmlrpc_c::serverAbyss* server_instance = nullptr;

void run_server(int port) {
    try {
        // Rule #6: xmlrpc-c's parser is considered safe against XXE by default.
        xmlrpc_c::registry myRegistry;

        // Rule #4: Map specific names to safe, pre-defined method handlers.
        xmlrpc_c::methodPtr const addMethodP(new sample_add_method);
        myRegistry.addMethod("sample.add", addMethodP);
        
        xmlrpc_c::methodPtr const divideMethodP(new sample_divide_method);
        myRegistry.addMethod("sample.divide", divideMethodP);

        std::cout << "Starting XML-RPC server on port " << port << std::endl;
        // Note: For production, configure HTTPS, not available in Abyss server directly.
        xmlrpc_c::serverAbyss myServer(
            xmlrpc_c::serverAbyss::constrOpt()
            .registryP(&myRegistry)
            .portNumber(port)
            .logFileName("/dev/null")); // Suppress log for clean output

        server_instance = &myServer;
        myServer.run(); // This call blocks until server is terminated.
        std::cout << "Server has been terminated." << std::endl;

    } catch (std::exception const& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}


void run_client_tests(int port) {
    // Give the server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try {
        std::cout << "\n--- Running Client Test Cases ---" << std::endl;
        std::string const serverUrl = "http://localhost:" + std::to_string(port) + "/RPC2";
        
        // Rule #1 & #2: xmlrpc_c client doesn't directly support HTTPS.
        // A proxy or different library would be needed for secure communication.
        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value result;

        // Test Case 1: Add
        myClient.call(serverUrl, "sample.add", "ii", &result, 10, 20);
        int sum = xmlrpc_c::value_int(result);
        std::cout << "Test 1: 10 + 20 = " << sum << std::endl;

        // Test Case 2: Subtract (using add with negative)
        myClient.call(serverUrl, "sample.add", "ii", &result, 100, -33);
        int diff = xmlrpc_c::value_int(result);
        std::cout << "Test 2: 100 - 33 = " << diff << std::endl;

        // Test Case 3: Divide
        myClient.call(serverUrl, "sample.divide", "dd", &result, 10.0, 4.0);
        double quot = xmlrpc_c::value_double(result);
        std::cout << "Test 3: 10.0 / 4.0 = " << quot << std::endl;

        // Test Case 4: Divide by zero (error case)
        try {
            myClient.call(serverUrl, "sample.divide", "dd", &result, 5.0, 0.0);
        } catch (xmlrpc_c::fault const& f) {
            std::cout << "Test 4: 5.0 / 0.0 -> Fault: " << f.getDescription() << std::endl;
        }

        // Test Case 5: Method not found (error case)
        try {
            myClient.call(serverUrl, "sample.multiply", "ii", &result, 5, 3);
        } catch (xmlrpc_c::fault const& f) {
            std::cout << "Test 5: call multiply -> Fault: " << f.getDescription() << std::endl;
        }

    } catch (std::exception const& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main() {
    int const port = 8080;

    std::thread server_thread(run_server, port);
    
    run_client_tests(port);

    if (server_instance) {
        std::cout << "\nShutting down the server." << std::endl;
        server_instance->terminate();
    }

    server_thread.join();

    return 0;
}