/*
 * This code requires the Crow C++ micro web framework.
 * Crow is header-only, so you just need to include its 'crow_all.h' header.
 * You also need to link against boost_system.
 *
 * How to compile:
 * 1. Download crow_all.h from https://github.com/CrowCpp/Crow/releases
 * 2. Place it in your include path.
 * 3. Install Boost libraries (e.g., `sudo apt-get install libboost-system-dev` on Debian/Ubuntu).
 * 4. Compile with: g++ -I/path/to/includes -o Lse82 Lse82.cpp -lboost_system -pthread
 *
 * Because of these external dependencies, a single C++ file is not 'drag-and-drop' runnable
 * without setting up the environment first.
 */
#include <iostream>
#include "crow_all.h" // Requires Crow C++ framework header

// Mock main function for demonstration if Crow is not available.
#if !defined(CROW_H)
int main() {
    std::cout << "Crow framework header not found." << std::endl;
    std::cout << "Please follow compilation instructions in the source code comments." << std::endl;
    return 1;
}
#else

// Main function to run the web application
int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api")
    ([](const crow::request& req){
        // 1. Get the payload parameter from the URL
        char* payload_raw = req.url_params.get("payload");
        if (payload_raw == nullptr) {
            return crow::response(400, "Payload parameter is missing");
        }
        std::string payload = payload_raw;

        // 2. Load the payload into a dictionary (JSON object)
        crow::json::rvalue data;
        try {
            data = crow::json::load(payload);
        } catch (const std::exception& e) {
            return crow::response(400, "Invalid JSON format in payload");
        }

        if (!data.has("name")) {
            return crow::response(400, "'name' key is missing");
        }

        if (data["name"].t() != crow::json::type::String) {
            return crow::response(400, "'name' value must be a string");
        }
        
        // 3. Return the name value from the dictionary
        std::string name = data["name"].s();
        return crow::response(200, name);
    });

    std::cout << "Server starting on port 18080" << std::endl;
    std::cout << "--- Test Cases ---" << std::endl;
    std::cout << "1. Success: curl 'http://localhost:18080/api?payload=%7B%22name%22%3A%22Alice%22%2C%22age%22%3A30%7D'" << std::endl;
    std::cout << "   Expected: Alice" << std::endl;
    std::cout << "2. Missing payload parameter: curl 'http://localhost:18080/api'" << std::endl;
    std::cout << "   Expected: 400 Bad Request - Payload parameter is missing" << std::endl;
    std::cout << "3. Invalid JSON in payload: curl 'http://localhost:18080/api?payload=invalid-json'" << std::endl;
    std::cout << "   Expected: 400 Bad Request - Invalid JSON format in payload" << std::endl;
    std::cout << "4. Missing 'name' key in payload: curl 'http://localhost:18080/api?payload=%7B%22age%22%3A30%7D'" << std::endl;
    std::cout << "   Expected: 400 Bad Request - 'name' key is missing" << std::endl;
    std::cout << "5. 'name' is not a string: curl 'http://localhost:18080/api?payload=%7B%22name%22%3A123%7D'" << std::endl;
    std::cout << "   Expected: 400 Bad Request - 'name' value must be a string" << std::endl;

    app.port(18080).multithreaded().run();
    
    return 0;
}
#endif