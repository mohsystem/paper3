
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

std::string sanitizeXPath(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (std::isalnum(c) || c == '_' || c == '-') {
            result += c;
        }
    }
    return result;
}

long getFileSize(const std::string& filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

std::string executeXPathQuery(const std::string& xpathValue, const std::string& xmlFileName) {
    if (xpathValue.empty()) {
        return "Error: XPath value is required";
    }
    if (xmlFileName.empty()) {
        return "Error: XML filename is required";
    }
    
    // Sanitize XPath input
    std::string sanitizedXPath = sanitizeXPath(xpathValue);
    
    // Check file existence and size
    std::ifstream file(xmlFileName);
    if (!file.good()) {
        return "Error: File does not exist";
    }
    
    long fileSize = getFileSize(xmlFileName);
    if (fileSize > MAX_FILE_SIZE) {
        return "Error: File size exceeds maximum allowed size";
    }
    
    // Initialize libxml2
    xmlInitParser();
    LIBXML_TEST_VERSION
    
    // Secure XML parsing - disable entity expansion
    xmlParserOption options = (xmlParserOption)(
        XML_PARSE_NOENT |
        XML_PARSE_DTDLOAD |
        XML_PARSE_NONET
    );
    
    xmlDocPtr doc = xmlReadFile(xmlFileName.c_str(), NULL, 
        XML_PARSE_NONET | XML_PARSE_NOENT);
    
    if (doc == NULL) {
        xmlCleanupParser();
        return "Error: Failed to parse XML file";
    }
    
    // Create XPath context
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return "Error: Failed to create XPath context";
    }
    
    // Construct XPath query
    std::string xpathQuery = "/tag[@id='" + sanitizedXPath + "']";
    
    // Execute XPath query
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
        (const xmlChar*)xpathQuery.c_str(), xpathCtx);
    
    std::stringstream result;
    
    if (xpathObj == NULL || xpathObj->nodesetval == NULL || 
        xpathObj->nodesetval->nodeNr == 0) {
        result << "No matching nodes found";
    } else {
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        for (int i = 0; i < nodes->nodeNr; i++) {
            xmlNodePtr node = nodes->nodeTab[i];
            xmlChar* content = xmlNodeGetContent(node);
            result << "Node " << (i + 1) << ": ";
            if (content != NULL) {
                result << content;
                xmlFree(content);
            }
            result << "\\n";
        }
    }
    
    // Cleanup
    if (xpathObj != NULL) xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    return result.str();
}

int main(int argc, char* argv[]) {
    std::cout << "=== Test Case 1: Valid input ===" << std::endl;
    std::cout << executeXPathQuery("test1", "test.xml") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Empty XPath ===" << std::endl;
    std::cout << executeXPathQuery("", "test.xml") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Non-existent file ===" << std::endl;
    std::cout << executeXPathQuery("test2", "nonexistent.xml") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Empty filename ===" << std::endl;
    std::cout << executeXPathQuery("test3", "") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Special characters in XPath ===" << std::endl;
    std::cout << executeXPathQuery("test'OR'1'='1", "test.xml") << std::endl;
    
    // Command line usage
    if (argc == 3) {
        std::cout << "\\n=== Command Line Execution ===" << std::endl;
        std::cout << executeXPathQuery(argv[1], argv[2]) << std::endl;
    }
    
    return 0;
}
