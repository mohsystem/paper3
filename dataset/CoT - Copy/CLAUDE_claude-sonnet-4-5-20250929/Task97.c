
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_PATH_LENGTH 1024
#define MAX_RESULT_LENGTH 4096

void sanitizeXPath(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < outputSize - 1; i++) {
        if (isalnum(input[i]) || input[i] == '_' || input[i] == '-') {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

long getFileSize(const char* filename) {
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

char* executeXPathQuery(const char* xpathValue, const char* xmlFileName) {
    static char result[MAX_RESULT_LENGTH];
    char sanitizedXPath[MAX_PATH_LENGTH];
    char xpathQuery[MAX_PATH_LENGTH];
    
    result[0] = '\\0';
    
    if (xpathValue == NULL || strlen(xpathValue) == 0) {
        snprintf(result, MAX_RESULT_LENGTH, "Error: XPath value is required");
        return result;
    }
    if (xmlFileName == NULL || strlen(xmlFileName) == 0) {
        snprintf(result, MAX_RESULT_LENGTH, "Error: XML filename is required");
        return result;
    }
    
    // Sanitize XPath input
    sanitizeXPath(xpathValue, sanitizedXPath, MAX_PATH_LENGTH);
    
    // Check file size
    long fileSize = getFileSize(xmlFileName);
    if (fileSize < 0) {
        snprintf(result, MAX_RESULT_LENGTH, "Error: File does not exist");
        return result;
    }
    if (fileSize > MAX_FILE_SIZE) {
        snprintf(result, MAX_RESULT_LENGTH, "Error: File size exceeds maximum allowed size");
        return result;
    }
    
    // Initialize libxml2
    xmlInitParser();
    LIBXML_TEST_VERSION
    
    // Secure XML parsing
    xmlDocPtr doc = xmlReadFile(xmlFileName, NULL, 
        XML_PARSE_NONET | XML_PARSE_NOENT);
    
    if (doc == NULL) {
        snprintf(result, MAX_RESULT_LENGTH, "Error: Failed to parse XML file");
        xmlCleanupParser();
        return result;
    }
    
    // Create XPath context
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        snprintf(result, MAX_RESULT_LENGTH, "Error: Failed to create XPath context");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return result;
    }
    
    // Construct XPath query
    snprintf(xpathQuery, MAX_PATH_LENGTH, "/tag[@id='%s']", sanitizedXPath);
    
    // Execute XPath query
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
        (const xmlChar*)xpathQuery, xpathCtx);
    
    if (xpathObj == NULL || xpathObj->nodesetval == NULL || 
        xpathObj->nodesetval->nodeNr == 0) {
        snprintf(result, MAX_RESULT_LENGTH, "No matching nodes found");
    } else {
        xmlNodeSetPtr nodes = xpathObj->nodesetval;
        int offset = 0;
        for (int i = 0; i < nodes->nodeNr && offset < MAX_RESULT_LENGTH - 100; i++) {
            xmlNodePtr node = nodes->nodeTab[i];
            xmlChar* content = xmlNodeGetContent(node);
            offset += snprintf(result + offset, MAX_RESULT_LENGTH - offset, 
                "Node %d: %s\\n", i + 1, content ? (char*)content : "");
            if (content != NULL) {
                xmlFree(content);
            }
        }
    }
    
    // Cleanup
    if (xpathObj != NULL) xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    return result;
}

int main(int argc, char* argv[]) {
    printf("=== Test Case 1: Valid input ===\\n");
    printf("%s\\n", executeXPathQuery("test1", "test.xml"));
    
    printf("\\n=== Test Case 2: Empty XPath ===\\n");
    printf("%s\\n", executeXPathQuery("", "test.xml"));
    
    printf("\\n=== Test Case 3: Non-existent file ===\\n");
    printf("%s\\n", executeXPathQuery("test2", "nonexistent.xml"));
    
    printf("\\n=== Test Case 4: NULL XPath ===\\n");
    printf("%s\\n", executeXPathQuery(NULL, "test.xml"));
    
    printf("\\n=== Test Case 5: Special characters in XPath ===\\n");
    printf("%s\\n", executeXPathQuery("test'OR'1'='1", "test.xml"));
    
    // Command line usage
    if (argc == 3) {
        printf("\\n=== Command Line Execution ===\\n");
        printf("%s\\n", executeXPathQuery(argv[1], argv[2]));
    }
    
    return 0;
}
