
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <tinyxml2.h>

using namespace std;
using namespace tinyxml2;

string executeXPathQuery(const string& xpathValue, const string& xmlFileName) {
    try {
        // Validate inputs
        if (xpathValue.empty()) {
            return "Error: XPath value cannot be empty";
        }
        if (xmlFileName.empty()) {
            return "Error: XML file name cannot be empty";
        }
        
        // Sanitize xpath value to prevent injection
        regex pattern("[^a-zA-Z0-9_-]");
        string sanitizedXPath = regex_replace(xpathValue, pattern, "");
        
        // Check if file exists
        ifstream fileCheck(xmlFileName);
        if (!fileCheck.good()) {
            return "Error: XML file not found or invalid";
        }
        fileCheck.close();
        
        // Parse XML file
        XMLDocument doc;
        XMLError result = doc.LoadFile(xmlFileName.c_str());
        
        if (result != XML_SUCCESS) {
            return "Error: Failed to parse XML file";
        }
        
        // Search for elements with matching id attribute
        string resultStr = "";
        XMLElement* root = doc.RootElement();
        
        function<void(XMLElement*)> searchElements = [&](XMLElement* element) {
            if (element == nullptr) return;
            
            const char* idAttr = element->Attribute("id");
            if (idAttr != nullptr && string(idAttr) == sanitizedXPath) {
                const char* text = element->GetText();
                resultStr += string(element->Name()) + ": " + 
                            (text ? text : "") + "\\n";
            }
            
            for (XMLElement* child = element->FirstChildElement(); 
                 child != nullptr; 
                 child = child->NextSiblingElement()) {
                searchElements(child);
            }
        };
        
        searchElements(root);
        
        if (!resultStr.empty()) {
            resultStr.pop_back(); // Remove trailing newline
            return resultStr;
        }
        
        return "No results found";
        
    } catch (const exception& e) {
        return string("Error: ") + e.what();
    }
}

int main() {
    // Test cases
    cout << "Test Case 1:" << endl;
    cout << executeXPathQuery("test1", "test.xml") << endl << endl;
    
    cout << "Test Case 2:" << endl;
    cout << executeXPathQuery("test2", "test.xml") << endl << endl;
    
    cout << "Test Case 3:" << endl;
    cout << executeXPathQuery("", "test.xml") << endl << endl;
    
    cout << "Test Case 4:" << endl;
    cout << executeXPathQuery("test1", "nonexistent.xml") << endl << endl;
    
    cout << "Test Case 5:" << endl;
    cout << executeXPathQuery("invalid-id", "test.xml") << endl;
    
    return 0;
}
