// This code requires the pugixml library.
// Compile with: g++ -std=c++17 your_script_name.cpp pugixml.cpp -o your_executable
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <regex>
#include <filesystem>

#include "pugixml.hpp"

// Rule #1: Regex for validating the ID to be alphanumeric + underscore
const std::regex ID_PATTERN("^[a-zA-Z0-9_]+$");

/**
 * @brief Executes an XPath query to find nodes by ID.
 * @param id The ID to search for. Must be a simple alphanumeric string.
 * @param filename The path to the XML file. Must be a simple filename without path components.
 * @return A vector of string contents of the found nodes.
 * @throws std::invalid_argument for invalid inputs.
 * @throws std::runtime_error on file or parsing errors.
 */
std::vector<std::string> executeXPathQuery(const std::string& id, const std::string& filename) {
    // Rule #1: Validate ID input
    if (!std::regex_match(id, ID_PATTERN)) {
        throw std::invalid_argument("Invalid ID format. Only alphanumeric characters and underscores are allowed.");
    }

    // Rule #5: Validate filename to prevent path traversal
    std::filesystem::path filePath(filename);
    if (filePath.filename().string() != filename) {
        throw std::invalid_argument("Invalid file path. Only simple filenames are allowed.");
    }
    if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
        throw std::runtime_error("File does not exist or is not a regular file: " + filename);
    }
    
    pugi::xml_document doc;
    // Rule #4: pugixml's default parser is safe and does not load external entities.
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (!result) {
        throw std::runtime_error("Error parsing XML file: " + std::string(result.description()));
    }

    std::vector<std::string> results;
    try {
        // Use parameterized queries to prevent XPath injection
        pugi::xpath_variable_set vars;
        vars.add("idVar", pugi::xpath_type_string);
        vars.set("idVar", id.c_str());

        // The format is /tag[@id={}], here we use `*` for any tag
        pugi::xpath_query query("//*[@id=$idVar]", &vars);
        pugi::xpath_node_set nodes = query.evaluate_node_set(doc);

        for (pugi::xpath_node node : nodes) {
            results.push_back(node.node().child_value());
        }
    } catch (const pugi::xpath_exception& e) {
        throw std::runtime_error("XPath evaluation failed: " + std::string(e.what()));
    }
    
    return results;
}

void setupTestFile(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<root>\n";
        file << "    <item id=\"item1\">Content 1</item>\n";
        file << "    <data>\n";
        file << "        <item id=\"item2\">Content 2</item>\n";
        file << "    </data>\n";
        file << "    <item id=\"item3\">Content 3</item>\n";
        file << "    <item id=\"another_id_4\">More Content</item>\n";
        file << "</root>";
        file.close();
    } else {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }
}

void cleanupTestFile(const std::string& filename) {
    std::filesystem::remove(filename);
}

void runTests() {
    std::string testFile = "test97.xml";
    try {
        setupTestFile(testFile);
        std::cout << "--- Running 5 Test Cases ---" << std::endl;

        // Test Case 1: Valid ID, existing item
        std::cout << "\n[Test 1] ID: 'item1', File: '" << testFile << "'" << std::endl;
        try {
            std::vector<std::string> res = executeXPathQuery("item1", testFile);
            std::cout << "Result: ";
            for(const auto& s : res) std::cout << s << " ";
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Test Case 2: Valid ID, nested item
        std::cout << "\n[Test 2] ID: 'item2', File: '" << testFile << "'" << std::endl;
        try {
            std::vector<std::string> res = executeXPathQuery("item2", testFile);
            std::cout << "Result: ";
            for(const auto& s : res) std::cout << s << " ";
            std::cout << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Test Case 3: Valid ID, non-existent item
        std::cout << "\n[Test 3] ID: 'nonexistent', File: '" << testFile << "'" << std::endl;
        try {
            std::vector<std::string> res = executeXPathQuery("nonexistent", testFile);
            std::cout << "Result: " << (res.empty() ? "(empty)" : "found items unexpectedly") << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Test Case 4: Invalid ID (potential injection)
        std::cout << "\n[Test 4] ID: \"' or '1'='1'\", File: '" << testFile << "'" << std::endl;
        try {
            executeXPathQuery("' or '1'='1'", testFile);
        } catch (const std::exception& e) {
            std::cerr << "Caught expected error: " << e.what() << std::endl;
        }

        // Test Case 5: Invalid filename (path traversal)
        std::cout << "\n[Test 5] ID: 'item1', File: '../test97.xml'" << std::endl;
        try {
            executeXPathQuery("item1", "../test97.xml");
        } catch (const std::exception& e) {
            std::cerr << "Caught expected error: " << e.what() << std::endl;
        }
        
        std::cout << "\n--- Test Cases Finished ---" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Test setup failed: " << e.what() << std::endl;
    }
    cleanupTestFile(testFile);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <id> <xml_file>" << std::endl;
        std::cerr << "Running test cases instead..." << std::endl;
        runTests();
        return 1;
    }

    try {
        std::vector<std::string> results = executeXPathQuery(argv[1], argv[2]);
        std::cout << "Query Result:" << std::endl;
        if (results.empty()) {
            std::cout << "No nodes found." << std::endl;
        } else {
            for (const auto& res : results) {
                std::cout << res << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}