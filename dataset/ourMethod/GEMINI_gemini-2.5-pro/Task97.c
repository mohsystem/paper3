#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#define MAX_FILE_SIZE 1048576 // 1 MB
#define MAX_ID_LENGTH 128
#define MAX_XPATH_LENGTH 256

// Forward declarations
static int is_valid_id(const char *id);
static int is_valid_filename(const char *filename);
static char* read_file_securely(const char *filename, long *file_size);
static int execute_xpath_query(const char* id_str, const char* xml_filename);

int main(int argc, char **argv) {
    if (argc == 3) {
        // Normal command-line execution
        LIBXML_TEST_VERSION;
        int result = execute_xpath_query(argv[1], argv[2]);
        xmlCleanupParser();
        return result;
    } else {
        // Run 5 test cases as requested if no CLI arguments are provided
        printf("Running built-in test cases...\n");
        printf("Usage: %s <id> <xml_file>\n", argv[0]);
        
        LIBXML_TEST_VERSION;

        // Test setup: create a dummy file
        const char* test_filename = "test_97.xml";
        FILE* f = fopen(test_filename, "w");
        if (f) {
            fprintf(f, "<?xml version=\"1.0\"?>\n<catalog>\n   <book id=\"bk101\">\n      <author>Gambardella, Matthew</author>\n      <title>XML Developer's Guide</title>\n   </book>\n   <book id=\"bk102\">\n      <author>Ralls, Kim</author>\n      <title>Midnight Rain</title>\n   </book>\n</catalog>");
            fclose(f);
        } else {
            perror("Failed to create test file");
            return EXIT_FAILURE;
        }
        
        // Test Case 1: Valid ID and File
        printf("\n--- Test Case 1: Valid ID 'bk101' and valid file '%s' ---\n", test_filename);
        execute_xpath_query("bk101", test_filename);

        // Test Case 2: ID not found in the XML
        printf("\n--- Test Case 2: ID 'bk999' not found in '%s' ---\n", test_filename);
        execute_xpath_query("bk999", test_filename);

        // Test Case 3: Invalid ID (containing characters for injection attempt)
        printf("\n--- Test Case 3: Invalid ID 'invalid'id' ---\n");
        execute_xpath_query("' or 1=1 or ''='", test_filename);

        // Test Case 4: Non-existent file
        printf("\n--- Test Case 4: Non-existent file 'nonexistent.xml' ---\n");
        execute_xpath_query("bk101", "nonexistent.xml");

        // Test Case 5: Invalid filename (with path traversal)
        printf("\n--- Test Case 5: Invalid filename '../test_97.xml' ---\n");
        execute_xpath_query("bk101", "../test_97.xml");

        // Test cleanup
        remove(test_filename);
        
        xmlCleanupParser();
        return EXIT_SUCCESS;
    }
}

static int is_valid_id(const char *id) {
    if (id == NULL) return 0;
    size_t len = strlen(id);
    if (len == 0 || len > MAX_ID_LENGTH) {
        return 0;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum((unsigned char)id[i]) && id[i] != '-' && id[i] != '_') {
            return 0; // Invalid character found
        }
    }
    return 1;
}

static int is_valid_filename(const char *filename) {
    if (filename == NULL || strlen(filename) == 0) {
        return 0;
    }
    if (strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        return 0;
    }
    return 1;
}

static char* read_file_securely(const char *filename, long *file_size) {
    int fd = -1;
    struct stat st;
    char *buffer = NULL;
    char *final_buffer = NULL;

    fd = open(filename, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        perror("Error opening file");
        return NULL;
    }

    if (fstat(fd, &st) < 0) {
        perror("Error getting file stats");
        close(fd);
        return NULL;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file: %s\n", filename);
        close(fd);
        return NULL;
    }

    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds limit of %d bytes.\n", MAX_FILE_SIZE);
        close(fd);
        return NULL;
    }

    *file_size = st.st_size;
    if (*file_size == 0) {
        buffer = malloc(1);
        if (buffer) {
            buffer[0] = '\0';
        }
        close(fd);
        return buffer;
    }

    buffer = malloc(*file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, buffer, *file_size);
    if (bytes_read < 0) {
        perror("Error reading file");
    } else if (bytes_read != *file_size) {
        fprintf(stderr, "Error: Incomplete read of file.\n");
    } else {
        buffer[*file_size] = '\0';
        final_buffer = buffer;
        buffer = NULL; // Prevent double free
    }

    free(buffer);
    close(fd);
    return final_buffer;
}

static int execute_xpath_query(const char* id_str, const char* xml_filename) {
    char *xml_buffer = NULL;
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr xpathCtx = NULL;
    xmlXPathObjectPtr xpathObj = NULL;
    int ret = EXIT_FAILURE;

    if (!is_valid_id(id_str)) {
        fprintf(stderr, "Error: Invalid ID format. Must be alphanumeric (-, _ allowed) and under %d chars.\n", MAX_ID_LENGTH);
        return EXIT_FAILURE;
    }
    if (!is_valid_filename(xml_filename)) {
        fprintf(stderr, "Error: Invalid filename. Must not contain path separators.\n");
        return EXIT_FAILURE;
    }

    long file_size;
    xml_buffer = read_file_securely(xml_filename, &file_size);
    if (!xml_buffer) {
        return EXIT_FAILURE;
    }

    // Securely parse the XML from memory
    doc = xmlReadMemory(xml_buffer, (int)file_size, xml_filename, NULL, XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (doc == NULL) {
        fprintf(stderr, "Error: Failed to parse XML file '%s'.\n", xml_filename);
        goto cleanup;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        fprintf(stderr, "Error: unable to create new XPath context\n");
        goto cleanup;
    }
    
    char xpath_expr[MAX_XPATH_LENGTH];
    int written = snprintf(xpath_expr, sizeof(xpath_expr), "//*[@id='%s']", id_str);
    if (written < 0 || (size_t)written >= sizeof(xpath_expr)) {
        fprintf(stderr, "Error: XPath expression is too long.\n");
        goto cleanup;
    }
    
    xpathObj = xmlXPathEvalExpression(BAD_CAST xpath_expr, xpathCtx);
    if (xpathObj == NULL) {
        fprintf(stderr, "Error: unable to evaluate xpath expression \"%s\"\n", xpath_expr);
        goto cleanup;
    }

    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    printf("XPath query '%s' result:\n", xpath_expr);
    if (nodes != NULL) {
        if (nodes->nodeNr == 0) {
            printf("No nodes found.\n");
        } else {
            for (int i = 0; i < nodes->nodeNr; ++i) {
                if (nodes->nodeTab[i] != NULL && nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
                    xmlChar* content = xmlNodeGetContent(nodes->nodeTab[i]);
                    if (content) {
                        printf("  Node[%d] Content: %s\n", i, content);
                        xmlFree(content);
                    }
                }
            }
        }
    } else {
        printf("XPath query did not return a nodeset.\n");
    }
    
    ret = EXIT_SUCCESS;

cleanup:
    if (xpathObj) xmlXPathFreeObject(xpathObj);
    if (xpathCtx) xmlXPathFreeContext(xpathCtx);
    if (doc) xmlFreeDoc(doc);
    free(xml_buffer);
    
    return ret;
}