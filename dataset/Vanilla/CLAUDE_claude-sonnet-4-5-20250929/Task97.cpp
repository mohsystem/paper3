
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Note: C++ doesn't have built-in XML/XPath support like Java or Python.\n// This is a simplified implementation that searches for id attributes.\n// For production use, consider libraries like libxml2, pugixml, or TinyXML.\n\nstd::string executeXPath(const std::string& xpathValue, const std::string& xmlFileName) {\n    std::ifstream file(xmlFileName);\n    if (!file.is_open()) {\n        return "Error: Could not open file";\n    }\n    \n    std::stringstream buffer;\n    buffer << file.rdbuf();\n    std::string xmlContent = buffer.str();\n    file.close();\n    \n    std::string result;\n    std::string searchPattern = "id=\\"" + xpathValue + "\\"";\n    std::string searchPattern2 = "id='" + xpathValue + "'";\n    \n    size_t pos = 0;\n    while ((pos = xmlContent.find(searchPattern, pos)) != std::string::npos ||\n           (pos = xmlContent.find(searchPattern2, pos)) != std::string::npos) {\n        \n        size_t tagStart = xmlContent.rfind('<', pos);\n        size_t tagNameEnd = xmlContent.find_first_of(" >", tagStart + 1);\n        std::string tagName = xmlContent.substr(tagStart + 1, tagNameEnd - tagStart - 1);\n        \n        size_t contentStart = xmlContent.find('>', pos) + 1;
        size_t contentEnd = xmlContent.find("</" + tagName, contentStart);
        std::string content = xmlContent.substr(contentStart, contentEnd - contentStart);
        
        result += tagName + ": " + content;
        pos = contentEnd;
        break;
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Test case 1
    std::cout << "Test Case 1:" << std::endl;
    std::cout << executeXPath("1", "test1.xml") << std::endl;
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test Case 2:" << std::endl;
    std::cout << executeXPath("2", "test2.xml") << std::endl;
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test Case 3:" << std::endl;
    std::cout << executeXPath("3", "test3.xml") << std::endl;
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test Case 4:" << std::endl;
    std::cout << executeXPath("item1", "test4.xml") << std::endl;
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test Case 5:" << std::endl;
    std::cout << executeXPath("main", "test5.xml") << std::endl;
    
    return 0;
}
