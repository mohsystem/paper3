/*
 * Requires the libxml2 library.
 * Install (on Debian/Ubuntu): sudo apt-get install libxml2-dev
 * Compile command: gcc Task97.c -o task_c `xml2-config --cflags --libs`
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/**
 * Executes a secure XPath query on a local XML file.
 *
 * @param id The ID value to be used in the XPath query.
 * @param filename The name of the XML file to query.
 * @return A dynamically allocated string with the result, or an error message.
 *         The caller is responsible for freeing the returned string.
 */
char* performXpathQuery(const char* id, const char* filename) {
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr context = NULL;
    xmlXPathObjectPtr result = NULL;
    char* output = NULL;

    // 1. Secure Filename Handling: Prevent path traversal
    if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        return strdup("Error: Invalid filename.");
    }
    
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return strdup("Error: File not found.");
    }
    fclose(f);

    // 2. Secure XML Parsing: Use XML_PARSE_NONET to prevent network access.
    // Modern libxml2 versions disable external entity loading by default.
    doc = xmlReadFile(filename, NULL, XML_PARSE_NONET);
    if (doc == NULL) {
        return strdup("Error: Failed to parse XML document.");
    }

    context = xmlXPathNewContext(doc);
    if (context == NULL) {
        output = strdup("Error: Failed to create XPath context.");
        goto cleanup;
    }

    // 3. Secure XPath Execution: Register a variable to prevent XPath injection
    // The BAD_CAST macro is used to cast strings to xmlChar*
    xmlChar* id_xml = xmlCharStrdup(id);
    if (xmlXPathRegisterVariable(context, BAD_CAST "idVar", xmlXPathNewString(id_xml)) != 0) {
        output = strdup("Error: Failed to register XPath variable.");
        xmlFree(id_xml);
        goto cleanup;
    }
    xmlFree(id_xml);

    // The query uses a variable ($idVar) instead of concatenating user input
    const xmlChar* xpathExpr = BAD_CAST "//tag[@id=$idVar]";
    result = xmlXPathEvalExpression(xpathExpr, context);
    if (result == NULL) {
        output = strdup("Error: Failed to evaluate XPath expression.");
        goto cleanup;
    }

    if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        output = strdup("Not found");
    } else {
        xmlNodeSetPtr nodes = result->nodesetval;
        // Get the content of the first node
        if (nodes->nodeTab[0]->children && nodes->nodeTab[0]->children->content) {
            output = (char*)xmlCharStrdup((const char*)nodes->nodeTab[0]->children->content);
        } else {
             output = strdup("");
        }
    }

cleanup:
    if (result) xmlXPathFreeObject(result);
    if (context) xmlXPathFreeContext(context);
    if (doc) xmlFreeDoc(doc);
    return output;
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
    FILE* outFile = fopen(testXmlFilename, "w");
     if (!outFile) {
        fprintf(stderr, "Failed to create test file.\n");
        return 1;
    }
    fprintf(outFile, "%s", xmlContent);
    fclose(outFile);
    
    // --- Test Cases ---
    const char* test_ids[] = {"1", "100", "999", "' or '1'='1", "admin"};
    const char* expected_results[] = {"First item", "Other item", "Not found", "Not found", "Admin item"};
    int num_tests = sizeof(test_ids) / sizeof(test_ids[0]);

    // Initialize the XML library
    xmlInitParser();

    printf("Running C Test Cases...\n");
    for (int i = 0; i < num_tests; ++i) {
        char* result = performXpathQuery(test_ids[i], testXmlFilename);
        const char* expected = expected_results[i];
        printf("Test %d: id='%s'\n  - Expected: %s\n  - Got: %s\n  - Status: %s\n\n",
               i + 1, test_ids[i], expected, result, strcmp(result, expected) == 0 ? "PASSED" : "FAILED");
        free(result); // Free the memory allocated by performXpathQuery
    }
    
    // Cleanup the library and test file
    xmlCleanupParser();
    remove(testXmlFilename);

    return 0;
}