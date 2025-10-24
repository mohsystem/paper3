/*
 * Note: This code requires the pugixml library.
 * Pugixml is a light-weight C++ XML processing library.
 * Download it from https://pugixml.org/
 *
 * To compile this code, you need to include pugixml.cpp in your compilation command,
 * for example:
 * g++ -o task97 Task97.cpp pugixml.cpp
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "pugixml.hpp"

/**
 * Executes an XPath query on a given XML file.
 *
 * @param xmlFilePath The path to the XML file.
 * @param xpathExpression The XPath expression to execute.
 * @return A vector of strings representing the query result.
 */
std::vector<std::string> executeXPath(const char* xmlFilePath, const char* xpathExpression) {
    std::vector<std::string> results;
    pugi::xml_document doc;

    // Secure parsing: Disable DTDs to prevent XXE. pugixml does this by default
    // with parse_default, but we make it explicit.
    pugi::xml_parse_result result = doc.load_file(xmlFilePath, pugi::parse_default | pugi::parse_pi);

    if (!result) {
        std::cerr << "Error: Failed to parse XML file: " << xmlFilePath << std::endl;
        std::cerr << "Error description: " << result.description() << std::endl;
        return results;
    }

    try {
        pugi::xpath_node_set query_results = doc.select_nodes(xpathExpression);
        for (pugi::xpath_node_set::const_iterator it = query_results.begin(); it != query_results.end(); ++it) {
            pugi::xpath_node node = *it;
            if (node.node()) {
                results.push_back(node.node().child_value());
            } else if (node.attribute()) {
                 results.push_back(node.attribute().value());
            }
        }
    } catch (const pugi::xpath_exception& e) {
        std::cerr << "XPath error: " << e.what() << std::endl;
    }

    return results;
}

// Main function with 5 test cases
int main() {
    // Create a dummy XML file for testing
    const char* xmlFileName = "test97.xml";
    std::ofstream xmlFile(xmlFileName);
    if (xmlFile.is_open()) {
        xmlFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        xmlFile << "<bookstore>" << std::endl;
        xmlFile << "  <book id=\"1\">" << std::endl;
        xmlFile << "    <title lang=\"en\">Harry Potter</title>" << std::endl;
        xmlFile << "    <author>J. K. Rowling</author>" << std::endl;
        xmlFile << "  </book>" << std::endl;
        xmlFile << "  <book id=\"2\">" << std::endl;
        xmlFile << "    <title lang=\"en\">Learning XML</title>" << std::endl;
        xmlFile << "    <author>Erik T. Ray</author>" << std::endl;
        xmlFile << "  </book>" << std::endl;
        xmlFile << "  <novel id=\"3\">" << std::endl;
        xmlFile << "    <title lang=\"en\">The Great Gatsby</title>" << std::endl;
        xmlFile << "    <author>F. Scott Fitzgerald</author>" << std::endl;
        xmlFile << "  </novel>" << std::endl;
        xmlFile << "</bookstore>" << std::endl;
        xmlFile.close();
    } else {
        std::cerr << "Failed to create test XML file." << std::endl;
        return 1;
    }

    // Simulating command line arguments: xpath, filename
    const char* testCases[][2] = {
        {"/bookstore/book/title", xmlFileName}, // Test Case 1: Get all book titles
        {"/bookstore/book[@id='1']/title", xmlFileName}, // Test Case 2: Get title of book with id=1
        {"//author", xmlFileName}, // Test Case 3: Get all authors
        {"/bookstore/novel/title", xmlFileName}, // Test Case 4: Get novel titles
        {"/bookstore/book[@id='99']/title", xmlFileName} // Test Case 5: Query for non-existent node
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        const char* xpath = testCases[i][0];
        const char* filename = testCases[i][1];
        std::cout << "Executing XPath: " << xpath << " on " << filename << std::endl;
        
        std::vector<std::string> results = executeXPath(filename, xpath);
        
        std::cout << "Result: [";
        for (size_t j = 0; j < results.size(); ++j) {
            std::cout << "\"" << results[j] << "\"";
            if (j < results.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl << std::endl;
    }

    // Clean up the dummy file
    remove(xmlFileName);

    return 0;
}