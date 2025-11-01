#include <iostream>
#include <string>
#include <regex>
#include <stdexcept>
// Note: This code requires the cpp-httplib library.
// It is a header-only library. Download "httplib.h" from its GitHub repository
// and place it in the same directory as this file or in your include path.
#include "httplib.h"

// The main logic is in the handler function passed to the server
void handle_request(const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("payload")) {
        res.set_content("Error: 'payload' parameter is missing.", "text/plain");
        res.status = 400;
        return;
    }

    std::string payload = req.get_param_value("payload");
    
    try {
        // Instead of a full YAML parser (which would be a heavy dependency),
        // we use a regex to check for the specific pattern. This is a compromise
        // for a single-file, dependency-light example.
        // This regex looks for 'type' followed by a colon and then 'Create',
        // allowing for whitespace.
        std::regex create_type_regex(R"(^\s*type\s*:\s*Create\s*$|\n\s*type\s*:\s*Create\s*$)", std::regex::ECMAScript);

        if (std::regex_search(payload, create_type_regex)) {
             res.set_content("Error: Payload with type 'Create' is not allowed.", "text/plain");
             res.status = 400;
        } else {
             res.set_content("OK: Payload processed successfully.", "text/plain");
             res.status = 200;
        }
    } catch (const std::exception& e) {
        res.set_content("Internal Server Error: " + std::string(e.what()), "text/plain");
        res.status = 500;
    }
}

int main() {
    httplib::Server svr;

    svr.Get("/", handle_request);

    int port = 5000;
    std::cout << "C++ server started on port " << port << std::endl;
    
    // --- 5 Test Cases (run in a separate terminal) ---
    std::cout << "\n--- Test Cases ---" << std::endl;
    std::cout << "1. No payload parameter (expects error):" << std::endl;
    std::cout << "   curl \"http://localhost:5000/\"" << std::endl;
    std::cout << "\n2. Payload with 'type: Create' (expects error):" << std::endl;
    std::cout << "   curl \"http://localhost:5000/?payload=type%3A%20Create\"" << std::endl;
    std::cout << "\n3. Payload with different type (expects OK):" << std::endl;
    std::cout << "   curl \"http://localhost:5000/?payload=type%3A%20Update\"" << std::endl;
    std::cout << "\n4. Payload with 'type: Create' and other data (expects error):" << std::endl;
    std::cout << "   curl \"http://localhost:5000/?payload=name%3A%20my-app%0Atype%3A%20Create\"" << std::endl;
    std::cout << "\n5. Valid YAML payload without 'type' key (expects OK):" << std::endl;
    std::cout << "   curl \"http://localhost:5000/?payload=name%3A%20my-app\"" << std::endl;
    std::cout << "\n--- Server is running. Press Ctrl+C to stop. ---" << std::endl;

    svr.listen("0.0.0.0", port);

    return 0;
}