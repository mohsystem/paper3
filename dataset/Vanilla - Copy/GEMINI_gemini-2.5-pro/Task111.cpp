/*
 * This code requires the XML-RPC for C/C++ (xmlrpc-c) library.
 * You need to install it first. On Debian/Ubuntu:
 * sudo apt-get install libxmlrpc-c++8-dev
 *
 * To compile and run:
 * g++ -o server.out your_script_name.cpp $(xmlrpc-c-config abyss-server --cflags --libs)
 * ./server.out
 *
 * A client can then connect to http://localhost:8082/RPC2
 */
#include <iostream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

// Test Case 1: Add two integers
class addMethod : public xmlrpc_c::method {
public:
    addMethod() {
        this->_signature = "i:ii";
        this->_help = "This method adds two integers.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value* const retvalP) {
        int const addend(paramList.getInt(0));
        int const adder(paramList.getInt(1));
        paramList.verifyEnd(2);
        std::cout << "Called add(" << addend << ", " << adder << ")" << std::endl;
        *retvalP = xmlrpc_c::value_int(addend + adder);
    }
};

// Test Case 2: Subtract two doubles
class subtractMethod : public xmlrpc_c::method {
public:
    subtractMethod() {
        this->_signature = "d:dd";
        this->_help = "This method subtracts two doubles.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value* const retvalP) {
        double const d1(paramList.getDouble(0));
        double const d2(paramList.getDouble(1));
        paramList.verifyEnd(2);
        std::cout << "Called subtract(" << d1 << ", " << d2 << ")" << std::endl;
        *retvalP = xmlrpc_c::value_double(d1 - d2);
    }
};

// Test Case 3: Greet a user
class greetMethod : public xmlrpc_c::method {
public:
    greetMethod() {
        this->_signature = "s:s";
        this->_help = "This method returns a greeting.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value* const retvalP) {
        std::string const name(paramList.getString(0));
        paramList.verifyEnd(1);
        std::cout << "Called greet(\"" << name << "\")" << std::endl;
        *retvalP = xmlrpc_c::value_string("Hello, " + name + "!");
    }
};

// Test Case 4: Return a struct
class getStructMethod : public xmlrpc_c::method {
public:
    getStructMethod() {
        this->_signature = "S:si";
        this->_help = "Returns a struct with user info.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value* const retvalP) {
        std::string const name(paramList.getString(0));
        int const age(paramList.getInt(1));
        paramList.verifyEnd(2);
        std::cout << "Called getStruct(\"" << name << "\", " << age << ")" << std::endl;

        std::map<std::string, xmlrpc_c::value> structData;
        structData["name"] = xmlrpc_c::value_string(name);
        structData["age"] = xmlrpc_c::value_int(age);
        structData["isAdult"] = xmlrpc_c::value_boolean(age >= 18);
        *retvalP = xmlrpc_c::value_struct(structData);
    }
};

// Test Case 5: Return an array
class getArrayMethod : public xmlrpc_c::method {
public:
    getArrayMethod() {
        this->_signature = "A:";
        this->_help = "Returns an array of strings.";
    }
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value* const retvalP) {
        paramList.verifyEnd(0);
        std::cout << "Called getArray()" << std::endl;
        
        std::vector<xmlrpc_c::value> arrayData;
        arrayData.push_back(xmlrpc_c::value_string("Java"));
        arrayData.push_back(xmlrpc_c::value_string("Python"));
        arrayData.push_back(xmlrpc_c::value_string("CPP"));
        arrayData.push_back(xmlrpc_c::value_string("C"));

        *retvalP = xmlrpc_c::value_array(arrayData);
    }
};

int main() {
    try {
        int port = 8082;
        xmlrpc_c::registry myRegistry;

        myRegistry.addMethod("add", new addMethod);
        myRegistry.addMethod("subtract", new subtractMethod);
        myRegistry.addMethod("greet", new greetMethod);
        myRegistry.addMethod("getStruct", new getStructMethod);
        myRegistry.addMethod("getArray", new getArrayMethod);
        
        xmlrpc_c::serverAbyss myAbyssServer(
            xmlrpc_c::serverAbyss::constrOpt()
            .registryP(&myRegistry)
            .portNumber(port));

        std::cout << "Starting XML-RPC server on port " << port << "..." << std::endl;
        std::cout << "Available methods: add, subtract, greet, getStruct, getArray" << std::endl;
        myAbyssServer.run();
        
        // This part is never reached because run() is an infinite loop
        std::cout << "Server has shut down." << std::endl;

    } catch (std::exception const& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}