/*
 * Note: This code requires the libxml2 library.
 * On Debian/Ubuntu, install with: sudo apt-get install libxml2-dev
 * On RedHat/CentOS, install with: sudo yum install libxml2-devel
 *
 * To compile this code, you need to link against libxml2:
 * gcc -o task97 Task97.c -lxml2 -I/usr/include/libxml2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlerror.h>


/**
 * Executes an XPath query on a given XML file.
 *
 * @param xmlFilePath The path to the XML file.
 * @param xpathExpression The XPath expression to execute.
 * @param result_count Pointer to an integer to store the number of results.
 * @return A dynamically allocated array of strings (char**) with the results.
 *         The caller is responsible for freeing this array and its contents.
 *         Returns NULL on failure.
 */
char** executeXPath(const char* xmlFilePath, const xmlChar* xpathExpression, int* result_count) {
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    char** results = NULL;
    *result_count = 0;

    // Secure parsing: XML_PARSE_NOENT prevents entity substitution (XXE)
    // XML_PARSE_NONET disables network access during parsing
    doc = xmlReadFile(xmlFilePath, NULL, XML_PARSE_NOENT | XML_PARSE_NONET);
    if (doc == NULL) {
        fprintf(stderr, "Error: unable to parse file \"%s\"\n", xmlFilePath);
        return NULL;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        fprintf(stderr, "Error: unable to create new XPath context\n");
        xmlFreeDoc(doc);
        return NULL;
    }

    xpathObj = xmlXPathEvalExpression(xpathExpression, xpathCtx);
    if (xpathObj == NULL) {
        fprintf(stderr, "Error: unable to evaluate xpath expression \"%s\"\n", xpathExpression);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    if (nodes != NULL) {
        *result_count = nodes->nodeNr;
        if (*result_count > 0) {
            results = (char**)malloc(sizeof(char*) * (*result_count));
            if (results == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
            } else {
                for (int i = 0; i < *result_count; ++i) {
                    xmlNodePtr currentNode = nodes->nodeTab[i];
                    // Get content of the node itself or its first text child
                    xmlChar* content = xmlNodeGetContent(currentNode);
                    if (content != NULL) {
                        results[i] = (char*)strdup((const char*)content);
                        xmlFree(content);
                    } else {
                        results[i] = strdup(""); // Handle empty nodes
                    }
                }
            }
        }
    }

    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    return results;
}

void free_results(char** results, int count) {
    if (results != NULL) {
        for (int i = 0; i < count; i++) {
            free(results[i]);
        }
        free(results);
    }
}

int main() {
    // Initialize the XML library
    xmlInitParser();

    // Create a dummy XML file for testing
    const char* xmlFileName = "test97.xml";
    FILE* fp = fopen(xmlFileName, "w");
    if (fp) {
        fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(fp, "<bookstore>\n");
        fprintf(fp, "  <book id=\"1\">\n");
        fprintf(fp, "    <title lang=\"en\">Harry Potter</title>\n");
        fprintf(fp, "    <author>J. K. Rowling</author>\n");
        fprintf(fp, "  </book>\n");
        fprintf(fp, "  <book id=\"2\">\n");
        fprintf(fp, "    <title lang=\"en\">Learning XML</title>\n");
        fprintf(fp, "    <author>Erik T. Ray</author>\n");
        fprintf(fp, "  </book>\n");
        fprintf(fp, "  <novel id=\"3\">\n");
        fprintf(fp, "    <title lang=\"en\">The Great Gatsby</title>\n");
        fprintf(fp, "    <author>F. Scott Fitzgerald</author>\n");
        fprintf(fp, "  </novel>\n");
        fprintf(fp, "</bookstore>");
        fclose(fp);
    } else {
        fprintf(stderr, "Failed to create test XML file.\n");
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
        printf("--- Test Case %d ---\n", i + 1);
        const char* xpath = testCases[i][0];
        const char* filename = testCases[i][1];
        printf("Executing XPath: %s on %s\n", xpath, filename);
        
        int count = 0;
        char** results = executeXPath(filename, (const xmlChar*)xpath, &count);
        
        printf("Result: [");
        if (results != NULL) {
            for (int j = 0; j < count; j++) {
                printf("\"%s\"", results[j]);
                if (j < count - 1) {
                    printf(", ");
                }
            }
        }
        printf("]\n\n");
        
        free_results(results, count);
    }

    // Clean up the dummy file and the library
    remove(xmlFileName);
    xmlCleanupParser();
    
    return 0;
}