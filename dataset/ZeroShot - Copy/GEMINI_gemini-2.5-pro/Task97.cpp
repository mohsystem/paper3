/*
 * Requires the pugixml library.
 * Download from https://pugixml.org/
 * For single-file compilation, place pugixml.hpp and pugixml.cpp in the same directory.
 * Compile command: g++ Task97.cpp pugixml.cpp -o task_cpp
 */
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "pugixml.hpp"

/**
 * Executes a secure XPath query on a local XML file.
 *
 * @param id The ID value to be used in the XPath query.
 * @param filename The name of the XML file to query.
 * @return The text content of the first matching node, or "Not found".
 */
std::string performXpathQuery(const std::string& id, const std::string& filename) {
    // 1. Secure Filename Handling: Prevent path traversal
    if (filename.find("..") != std::string::npos || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        return "Error: Invalid filename.";
    }

    std::ifstream file(filename);
    if (!file.good()) {
        return "Error: File not found.";
    }

    // 2. Secure XML Parsing: pugixml is secure by default (no DTD/external entity loading)
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result) {
        return "Error: XML parsing failed.";
    }

    // 3. Secure XPath Execution: Use variables to prevent XPath injection
    pugi::xpath_variable_set vars;
    vars.add("idVar", pugi::xpath_type_string, id.c_str());

    try {
        // The query uses a variable ($idVar) instead of concatenating user input
        pugi::xpath_query query("//tag[@id=$idVar]", &vars);
        pugi::xpath_node node = query.evaluate_node(doc);

        if (node) {
            return node.node().child_value();
        } else {
            return "Not found";
        }
    } catch (const pugi::xpath_exception& e) {
        return std::string("Error: XPath execution failed - ") + e.what();
    }
}


int main() {
    // Create a dummy XML file for testing
    const char* testXmlFilename = "test97.xml";
    const char* xmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                            "<root>\n"
                            "    <data>\n"
                            "        <tag id=\"1\">First item</tag>\n"
                            "        <tag id=\"2\">Second item</tag>\n"
                            "        <tag id=\"admin\">Admin item</tag>\n"
                            "    </data>\n"
                            "    <other>\n"
                            "        <tag id=\"100\">Other item</tag>\n"
                            "    </other>\n"
                            "</root>";
    std::ofstream outFile(testXmlFilename);
    if (!outFile) {
        std::cerr << "Failed to create test file." << std::endl;
        return 1;
    }
    outFile << xmlContent;
    outFile.close();

    // --- Test Cases ---
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"1", "First item"},
        {"100", "Other item"},
        {"999", "Not found"},
        {"' or '1'='1", "Not found"}, // Malicious input for injection test
        {"admin", "Admin item"}
    };

    std::cout << "Running C++ Test Cases..." << std::endl;
    int testNum = 1;
    for (const auto& testCase : testCases) {
        const std::string& id = testCase.first;
        const std::string& expected = testCase.second;
        std::string result = performXpathQuery(id, testXmlFilename);
        std::cout << "Test " << testNum++ << ": id='" << id << "'\n"
                  << "  - Expected: " << expected << "\n"
                  << "  - Got: " << result << "\n"
                  << "  - Status: " << (expected == result ? "PASSED" : "FAILED") << "\n" << std::endl;
    }
    
    // Cleanup the test file
    remove(testXmlFilename);

    return 0;
}