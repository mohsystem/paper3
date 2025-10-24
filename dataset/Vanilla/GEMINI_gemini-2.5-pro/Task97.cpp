// This code requires the pugixml library.
// Download it from https://pugixml.org/
// To compile: g++ your_file.cpp pugixml.cpp -o your_app
// Make sure pugixml.hpp and pugixml.cpp are in the same directory or in your include path.
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio> // For std::remove
#include "pugixml.hpp"

/**
 * Executes an XPath query on a given XML file using pugixml.
 *
 * @param xmlFileName The path to the XML file.
 * @param xpathExpression The XPath expression to evaluate.
 * @return A string representation of the result, or an error message.
 */
std::string evaluateXPath(const std::string& xmlFileName, const std::string& xpathExpression) {
    pugi::xml_document doc;
    pugi::xml_parse_result parse_result = doc.load_file(xmlFileName.c_str());

    if (!parse_result) {
        return "Error: XML file parsing failed - " + std::string(parse_result.description());
    }

    try {
        pugi::xpath_node_set nodes = doc.select_nodes(xpathExpression.c_str());
        
        if (!nodes.empty()) {
            std::stringstream ss;
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (i > 0) {
                    ss << ", ";
                }
                pugi::xpath_node node = nodes[i];
                if (node.attribute()) {
                    ss << node.attribute().value();
                } else if (node.node()) {
                    // For text() nodes
                    if (node.node().type() == pugi::node_pcdata) {
                        ss << node.node().value();
                    } else { // For element nodes, get concatenated child text
                        ss << node.node().child_value();
                    }
                }
            }
            return ss.str();
        } else {
            // Fallback for scalar types like count() or string results
            // select_string returns number for count() or string value for first node
            return doc.select_string(xpathExpression.c_str());
        }
    } catch (const pugi::xpath_exception& e) {
        return "Error: XPath evaluation failed - " + std::string(e.what());
    }
}

int main() {
    const std::string xmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<bookstore>\n"
        "    <book id=\"1\">\n"
        "        <title>The Lord of the Rings</title>\n"
        "        <author>J.R.R. Tolkien</author>\n"
        "        <year>1954</year>\n"
        "    </book>\n"
        "    <book id=\"2\">\n"
        "        <title>Pride and Prejudice</title>\n"
        "        <author>Jane Austen</author>\n"
        "        <year>1813</year>\n"
        "    </book>\n"
        "    <section name=\"fantasy\">\n"
        "         <book id=\"3\">\n"
        "            <title>A Game of Thrones</title>\n"
        "            <author>George R. R. Martin</author>\n"
        "            <year>1996</year>\n"
        "        </book>\n"
        "    </section>\n"
        "    <book id=\"4\">\n"
        "        <title>The Hobbit</title>\n"
        "        <author>J.R.R. Tolkien</author>\n"
        "        <year>1937</year>\n"
        "    </book>\n"
        "</bookstore>";
    const std::string fileName = "test.xml";

    std::ofstream outFile(fileName);
    if (!outFile.is_open()) {
        std::cerr << "Failed to create test file." << std::endl;
        return 1;
    }
    outFile << xmlContent;
    outFile.close();

    std::vector<std::pair<std::string, std::string>> testCases = {
        {"/bookstore/book[@id='1']/title/text()", "The Lord of the Rings"},
        {"//book[@id='3']/author/text()", "George R. R. Martin"},
        {"/bookstore/book[author='J.R.R. Tolkien']/title/text()", "The Lord of the Rings, The Hobbit"},
        {"count(//book)", "4"},
        {"//section/@name", "fantasy"}
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& testCase : testCases) {
        std::string xpath = testCase.first;
        std::string expected = testCase.second;
        std::string result = evaluateXPath(fileName, xpath);
        std::cout << "XPath: " << xpath << std::endl;
        std::cout << "Result: " << result << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        std::cout << "Status: " << (expected == result ? "Passed" : "Failed") << std::endl;
        std::cout << std::endl;
    }

    std::remove(fileName.c_str());

    return 0;
}