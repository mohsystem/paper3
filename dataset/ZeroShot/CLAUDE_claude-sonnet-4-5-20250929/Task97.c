
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

char* executeXPathQuery(const char* xpathValue, const char* xmlFileName) {
    char* result = (char*)malloc(1024);
    if (!result) return NULL;
    
    // Validate inputs
    if (!xpathValue || strlen(xpathValue) == 0) {
        strcpy(result, "Error: XPath value cannot be empty");
        return result;
    }
    if (!xmlFileName || strlen(xmlFileName) == 0) {
        strcpy(result, "Error: XML file name cannot be empty");
        return result;
    }
    
    // Sanitize xpath value
    char sanitizedXPath[256] = {0};
    int j = 0;
    for (int i = 0; i < strlen(xpathValue) && j < 255; i++) {
        if (isalnum(xpathValue[i]) || xpathValue[i] == '_' || xpathValue[i] == '-') {
            sanitizedXPath[j++] = xpathValue[i];
        }
    }
    sanitizedXPath[j] = '\\0';
    
    // Parse XML file
    xmlDocPtr doc = xmlParseFile(xmlFileName);
    if (!doc) {
        strcpy(result, "Error: Failed to parse XML file");
        return result;
    }
    
    // Create XPath context
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    if (!xpathCtx) {
        xmlFreeDoc(doc);
        strcpy(result, "Error: Failed to create XPath context");
        return result;
    }
    
    // Build XPath expression
    char xpathExpr[512];
    snprintf(xpathExpr, sizeof(xpathExpr), "//*[@id='%s']", sanitizedXPath);
    
    // Execute XPath query
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar*)xpathExpr, xpathCtx);
    
    if (!xpathObj) {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        strcpy(result, "Error: Failed to execute XPath query");
        return result;
    }
    
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    result[0] = '\\0';
    
    if (nodes && nodes->nodeNr > 0) {
        for (int i = 0; i < nodes->nodeNr; i++) {
            xmlNodePtr node = nodes->nodeTab[i];
            xmlChar* content = xmlNodeGetContent(node);
            
            char temp[256];
            snprintf(temp, sizeof(temp), "%s: %s\\n", node->name, content ? (char*)content : "");
            strcat(result, temp);
            
            if (content) xmlFree(content);
        }
        result[strlen(result) - 1] = '\\0'; // Remove trailing newline
    } else {
        strcpy(result, "No results found");
    }
    
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    return result;
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    char* res1 = executeXPathQuery("test1", "test.xml");
    printf("%s\\n\\n", res1);
    free(res1);
    
    printf("Test Case 2:\\n");
    char* res2 = executeXPathQuery("test2", "test.xml");
    printf("%s\\n\\n", res2);
    free(res2);
    
    printf("Test Case 3:\\n");
    char* res3 = executeXPathQuery("", "test.xml");
    printf("%s\\n\\n", res3);
    free(res3);
    
    printf("Test Case 4:\\n");
    char* res4 = executeXPathQuery("test1", "nonexistent.xml");
    printf("%s\\n\\n", res4);
    free(res4);
    
    printf("Test Case 5:\\n");
    char* res5 = executeXPathQuery("invalid-id", "test.xml");
    printf("%s\\n", res5);
    free(res5);
    
    return 0;
}
