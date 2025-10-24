/*
NOTE: This code requires the 'xmlrpc-c' library. It cannot be compiled
as a single file without it.

On Debian/Ubuntu, install the library:
sudo apt-get install libxmlrpc-c++8-dev

To compile:
g++ -o server Task111.cpp -lxmlrpc_server_abyss++ -lxmlrpc_server++ -lxmlrpc_server -lxmlrpc++ -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok

This program starts a server and waits for requests. There are no client
test cases in this file. You can use the provided Python client script below
to test the running server.

--- Python Test Client (save as client.py) ---
import xmlrpc.client

proxy = xmlrpc.client.ServerProxy("http://localhost:8080/RPC2")

# Test Case 1
r1 = proxy.sample.add(5, 3)
print(f"Test 1: sample.add(5, 3) = {r1}")

# Test Case 2
r2 = proxy.sample.add(-10, 5)
print(f"Test 2: sample.add(-10, 5) = {r2}")

# Test Case 3
r3 = proxy.sample.add(0, 0)
print(f"Test 3: sample.add(0, 0) = {r3}")

# Test Case 4
r4 = proxy.sample.add(12345, 67890)
print(f"Test 4: sample.add(12345, 67890) = {r4}")

# Test Case 5
r5 = proxy.sample.subtract(10, 4)
print(f"Test 5: sample.subtract(10, 4) = {r5}")
------------------------------------------------
*/
#include <iostream>
#include <stdexcept>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

class sampleAddMethod : public xmlrpc_c::method {
public:
    sampleAddMethod() {
        this->_signature = "i:ii"; // Returns integer, takes two integers
        this->_help = "This method adds two integers.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value* const retvalP) {
        
        paramList.verifyEnd(2); // Ensure exactly 2 parameters
        int const addend(paramList.getInt(0));
        int const adder(paramList.getInt(1));
        
        *retvalP = xmlrpc_c::value_int(addend + adder);
    }
};

class sampleSubtractMethod : public xmlrpc_c::method {
public:
    sampleSubtractMethod() {
        this->_signature = "i:ii"; // Returns integer, takes two integers
        this->_help = "This method subtracts the second integer from the first.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value* const retvalP) {

        paramList.verifyEnd(2); // Ensure exactly 2 parameters
        int const minuend(paramList.getInt(0));
        int const subtrahend(paramList.getInt(1));
        
        *retvalP = xmlrpc_c::value_int(minuend - subtrahend);
    }
};

int main() {
    const int port = 8080;

    try {
        // A registry is a collection of methods the server can execute
        xmlrpc_c::registry myRegistry;

        // Create method objects
        xmlrpc_c::methodPtr const addMethodP(new sampleAddMethod);
        xmlrpc_c::methodPtr const subtractMethodP(new sampleSubtractMethod);

        // Add methods to the registry
        myRegistry.addMethod("sample.add", addMethodP);
        myRegistry.addMethod("sample.subtract", subtractMethodP);

        // Set up and run the server
        xmlrpc_c::serverAbyss myAbyssServer(
            xmlrpc_c::serverAbyss::constrOpt()
            .registryP(&myRegistry)
            .portNumber(port)
            .logFileName("/dev/null")); // Suppress logging for simplicity

        std::cout << "XML-RPC server listening on port " << port << "..." << std::endl;
        std::cout << "Press Ctrl-C to terminate." << std::endl;
        
        myAbyssServer.run();

        // The server's 'run' method never returns, so this is unreachable.
        // It runs until the process is killed.
        
    } catch (std::exception const& e) {
        std::cerr << "Server failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}