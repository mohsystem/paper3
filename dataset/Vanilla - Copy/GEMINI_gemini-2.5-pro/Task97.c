// This code requires the libxml2 library.
// It is pre-installed on many Linux distributions.
// To compile: gcc your_file.c -o your_app `xml2-config --cflags --libs`
// Or: gcc your_file.c -o your_app -I/usr/include/libxml2 -lxml2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlstring.h>

/**
 * Executes an XPath query on a given XML file using libxml2.
 * NOTE: The caller is responsible for freeing the returned string.
 *
 * @param xmlFileName The path to the XML file.
 * @param xpathExpression The XPath expression to evaluate.
 * @return A dynamically allocated string with the result, or an error message.
 */
char* evaluateXPath(const char* xmlFileName, const char* xpathExpression) {
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;

    doc = xmlReadFile(xmlFileName, NULL, 0);
    if (doc == NULL) {
        return strdup("Error: Unable to parse XML file.");
    }

    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        xmlFreeDoc(doc);
        return strdup("Error: Unable to create new XPath context.");
    }

    xpathObj = xmlXPathEvalExpression((const xmlChar*)xpathExpression, xpathCtx);
    if (xpathObj == NULL) {
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return strdup("Error: Unable to evaluate XPath expression.");
    }

    char* result_buffer = NULL;
    size_t buffer_size = 0;
    size_t current_len = 0;

    switch (xpathObj->type) {
        case XPATH_NODESET: {
            xmlNodeSetPtr nodes = xpathObj->nodesetval;
            if (nodes != NULL) {
                for (int i = 0; i < nodes->nodeNr; i++) {
                    // xmlNodeGetContent handles text nodes, attributes, etc.
                    xmlChar* content = xmlNodeGetContent(nodes->nodeTab[i]);
                    if (content) {
                        size_t content_len = strlen((char*)content);
                        size_t separator_len = (i > 0) ? 2 : 0;
                        result_buffer = realloc(result_buffer, current_len + content_len + separator_len + 1);
                        if (i > 0) {
                            strcpy(result_buffer + current_len, ", ");
                        }
                        strcpy(result_buffer + current_len + separator_len, (char*)content);
                        current_len += content_len + separator_len;
                        xmlFree(content);
                    }
                }
            }
            break;
        }
        case XPATH_NUMBER:
            // Allocate a buffer large enough for a double
            result_buffer = malloc(64);
            sprintf(result_buffer, "%g", xpathObj->floatval);
            // Remove ".0" for integer values to match other examples
            char *p = strchr(result_buffer, '.');
            if (p && strlen(p) == 2 && *(p+1) == '0') {
                *p = '\0';
            }
            break;
        case XPATH_STRING:
            result_buffer = strdup((char*)xpathObj->stringval);
            break;
        default:
            break; // Other types not handled, will return NULL or empty string
    }

    // If no result was generated, create an empty string
    if (result_buffer == NULL) {
        result_buffer = strdup("");
    }
    
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);

    return result_buffer;
}

int main() {
    const char* xmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
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
    const char* fileName = "test.xml";

    FILE* fp = fopen(fileName, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create test file.\n");
        return 1;
    }
    fputs(xmlContent, fp);
    fclose(fp);

    // This should be called once at the beginning of the program
    xmlInitParser();

    const char* testCases[][2] = {
        {"/bookstore/book[@id='1']/title/text()", "The Lord of the Rings"},
        {"//book[@id='3']/author/text()", "George R. R. Martin"},
        {"/bookstore/book[author='J.R.R. Tolkien']/title/text()", "The Lord of the Rings, The Hobbit"},
        {"count(//book)", "4"},
        {"//section/@name", "fantasy"}
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; ++i) {
        const char* xpath = testCases[i][0];
        const char* expected = testCases[i][1];
        char* result = evaluateXPath(fileName, xpath);
        
        printf("XPath: %s\n", xpath);
        printf("Result: %s\n", result);
        printf("Expected: %s\n", expected);
        printf("Status: %s\n", strcmp(expected, result) == 0 ? "Passed" : "Failed");
        printf("\n");

        free(result); // Free the memory allocated by evaluateXPath
    }

    // Cleanup libxml2 parser
    xmlCleanupParser();

    remove(fileName);

    return 0;
}