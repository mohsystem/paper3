
#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <map>
#include <httplib.h>

const size_t MAX_PAYLOAD_SIZE = 10 * 1024; // 10KB

class Lse83 {
private:
    static std::string escapeJson(const std::string& input) {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '\\\\': output += "\\\\\\\\"; break;\n                case '"': output += "\\\\\\""; break;
                case '\\n': output += "\\\\n"; break;
                case '\\r': output += "\\\\r"; break;
                case '\\t': output += "\\\\t"; break;
                default: output += c; break;
            }
        }
        return output;
    }

    static std::string extractJsonValue(const std::string& json, const std::string& key) {
        std::regex pattern("\\"" + key + "\\"\\\\s*:\\\\s*\\"([^\\"]+)\\"");
        std::smatch match;
        if (std::regex_search(json, match, pattern)) {
            return match[1].str();
        }
        return "";
    }

public:
    static std::string processPayload(const std::string& jsonPayload) {
        if (jsonPayload.empty()) {
            return "{\\"error\\": \\"Empty payload\\"}";
        }

        if (jsonPayload.length() > MAX_PAYLOAD_SIZE) {
            return "{\\"error\\": \\"Payload too large\\"}";
        }

        // Check if JSON contains "name" field
        if (jsonPayload.find("\\"name\\"") == std::string::npos) {
            return "{\\"error\\": \\"Name field not found\\"}";
        }

        std::string name = extractJsonValue(jsonPayload, "name");

        if (name.empty()) {
            return "{\\"error\\": \\"Name field is empty\\"}";
        }

        // Validate name format (alphanumeric, spaces, hyphens, underscores, dots)
        std::regex safePattern("^[a-zA-Z0-9\\\\s\\\\-_.]{1,100}$");
        if (!std::regex_match(name, safePattern)) {
            return "{\\"error\\": \\"Invalid name format\\"}";
        }

        std::string sanitizedName = escapeJson(name);
        return "{\\"name\\": \\"" + sanitizedName + "\\"}";
    }

    static void startWebServer() {
        httplib::Server svr;

        svr.Post("/api/payload", [](const httplib::Request& req, httplib::Response& res) {
            std::string response = processPayload(req.body);
            res.set_content(response, "application/json");
        });

        std::cout << "Server started on port 8000" << std::endl;
        svr.listen("0.0.0.0", 8000);
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1: " << Lse83::processPayload("{\\"name\\": \\"John Doe\\"}") << std::endl;
    std::cout << "Test Case 2: " << Lse83::processPayload("{\\"age\\": 30}") << std::endl;
    std::cout << "Test Case 3: " << Lse83::processPayload("{\\"name\\": \\"\\"}") << std::endl;
    std::cout << "Test Case 4: " << Lse83::processPayload("{}") << std::endl;
    std::cout << "Test Case 5: " << Lse83::processPayload("{\\"name\\": \\"Alice_123\\"}") << std::endl;

    // Note: To run the web server, uncomment the line below and include cpp-httplib library
    // Lse83::startWebServer();

    return 0;
}
