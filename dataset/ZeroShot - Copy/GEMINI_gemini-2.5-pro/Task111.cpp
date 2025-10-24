/*
 * This code requires the xmlrpc-c library.
 * On Debian/Ubuntu: sudo apt-get install libxmlrpc-c++8-dev
 * On RedHat/CentOS: sudo yum install xmlrpc-c-devel
 *
 * To compile:
 * g++ your_file_name.cpp -o server_cpp $(xmlrpc-c-config abyss-server --cflags --libs)
 *
 * Then run:
 * ./server_cpp
 *
 * NOTE: This is a C++ implementation.
 * The main class is named Task111 to follow the prompt's instructions,
 * though classes are used differently in C++ than in Java.
*/
#include <iostream>
#include <string>
#include <stdexcept>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

using namespace std;

class sampleAddMethod : public xmlrpc_c::method {
public:
    sampleAddMethod() {
        // signature and help strings are documentation -- not enforced.
        this->_signature = "i:ii";
        this->_help = "This method adds two integers.";
    }
    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {
        
        // Secure: Check parameter count
        if (paramList.size() != 2) {
            throw xmlrpc_c::fault("Incorrect number of parameters for 'add'. Expected 2.",
                                  xmlrpc_c::fault::CODE_TYPE);
        }

        // Secure: Validate types during unpacking. paramList.getInt will throw if type is wrong.
        int const addend(paramList.getInt(0));
        int const adder(paramList.getInt(1));
        paramList.verifyEnd(2);

        // Secure: Check for integer overflow before performing the operation
        if ((adder > 0 && addend > INT_MAX - adder) || (adder < 0 && addend < INT_MIN - adder)) {
            throw xmlrpc_c::fault("Integer overflow detected.", xmlrpc_c::fault::CODE_INTERNAL);
        }

        *retvalP = xmlrpc_c::value_int(addend + adder);
    }
};

class sampleSubtractMethod : public xmlrpc_c::method {
public:
    sampleSubtractMethod() {
        this->_signature = "i:ii";
        this->_help = "This method subtracts the second integer from the first.";
    }
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP) {
        paramList.verifyEnd(2);
        int const minuend = paramList.getInt(0);
        int const subtrahend = paramList.getInt(1);

        if ((subtrahend > 0 && minuend < INT_MIN + subtrahend) || (subtrahend < 0 && minuend > INT_MAX + subtrahend)) {
            throw xmlrpc_c::fault("Integer overflow detected.", xmlrpc_c::fault::CODE_INTERNAL);
        }
        *retvalP = xmlrpc_c::value_int(minuend - subtrahend);
    }
};

class sampleDivideMethod : public xmlrpc_c::method {
public:
    sampleDivideMethod() {
        this->_signature = "d:dd";
        this->_help = "This method divides the first double by the second.";
    }
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP) {
        paramList.verifyEnd(2);
        double const dividend = paramList.getDouble(0);
        double const divisor = paramList.getDouble(1);

        if (divisor == 0.0) {
            throw xmlrpc_c::fault("Division by zero.", xmlrpc_c::fault::CODE_INTERNAL);
        }
        *retvalP = xmlrpc_c::value_double(dividend / divisor);
    }
};


class Task111 {
public:
    static void run(int argc, char **argv) {
        int port = 8082;
        if (argc > 1) {
            port = atoi(argv[1]);
        }

        try {
            // A registry is a list of methods the server can execute
            xmlrpc_c::registry myRegistry;

            // Add our methods to the registry
            xmlrpc_c::methodPtr const addMethodP(new sampleAddMethod);
            myRegistry.addMethod("sample.add", addMethodP);

            xmlrpc_c::methodPtr const subtractMethodP(new sampleSubtractMethod);
            myRegistry.addMethod("sample.subtract", subtractMethodP);
            
            xmlrpc_c::methodPtr const divideMethodP(new sampleDivideMethod);
            myRegistry.addMethod("sample.divide", divideMethodP);

            // Set up and run the server
            // For security, you would run this behind a reverse proxy (e.g. Nginx) with TLS.
            xmlrpc_c::serverAbyss myAbyssServer(
                xmlrpc_c::serverAbyss::constrOpt()
                .registryP(&myRegistry)
                .portNumber(port)
                .logFileName("/tmp/xmlrpc_log") // For production, use a proper logging setup
            );
            
            cout << "Starting XML-RPC C++ server on port " << port << "..." << endl;
            cout << "Accepting requests. (Press Ctrl-C to stop)" << endl;
            
            cout << "\n--- Test Cases (run from a separate client) ---" << endl;
            cout << "You can use a Python client for testing:" << endl;
            cout << "----------------------------------------------" << endl;
            cout << "import xmlrpc.client" << endl;
            cout << "s = xmlrpc.client.ServerProxy('http://localhost:8082/RPC2')" << endl;
            cout << "# Test Case 1: Add" << endl;
            cout << "print(s.sample.add(100, 200))" << endl;
            cout << "# Test Case 2: Subtract" << endl;
            cout << "print(s.sample.subtract(100, 75))" << endl;
            cout << "# Test Case 3: Divide" << endl;
            cout << "print(s.sample.divide(10.0, 4.0))" << endl;
            cout << "# Test Case 4: Divide by zero (error handling)" << endl;
            cout << "try:\n    print(s.sample.divide(10, 0))\nexcept Exception as e:\n    print(e)" << endl;
            cout << "# Test Case 5: Incorrect parameter type (error handling)" << endl;
            cout << "try:\n    print(s.sample.add('a', 'b'))\nexcept Exception as e:\n    print(e)" << endl;
            cout << "----------------------------------------------" << endl;

            myAbyssServer.run();
            // run() never returns
            
        } catch (exception const& e) {
            cerr << "Something failed.  " << e.what() << endl;
        }
    }
};


int main(int argc, char **argv) {
    Task111::run(argc, argv);
    return 0;
}