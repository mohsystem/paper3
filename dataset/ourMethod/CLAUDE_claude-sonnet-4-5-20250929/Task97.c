
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB limit
#define MAX_XPATH_LEN 1024
#define MAX_ID_LEN 256

// Secure zeroization for sensitive data
static void explicit_zero(void *ptr, size_t len) {
    volatile unsigned char *p = ptr;
    while (len--) *p++ = 0;
}

// Safe string copy with bounds checking
static int safe_strncpy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) return -1;
    size_t len = strnlen(src, dest_size);
    if (len >= dest_size) return -1;
    memcpy(dest, src, len);
    dest[len] = '\\0';
    return 0;
}

// Sanitize ID value to prevent XPath injection
static int sanitize_id(const char *input, char *output, size_t out_size) {
    if (!input || !output || out_size < 2) return -1;
    
    size_t in_len = strnlen(input, MAX_ID_LEN);
    if (in_len == 0 || in_len >= MAX_ID_LEN) return -1;
    
    size_t out_idx = 0;
    for (size_t i = 0; i < in_len && out_idx < out_size - 1; i++) {
        char c = input[i];
        // Allow only alphanumeric, underscore, hyphen, and period
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.') {
            output[out_idx++] = c;
        } else {
            return -1; // Reject invalid characters
        }
    }
    output[out_idx] = '\\0';
    return 0;
}

// Safe file read with race-safe validation
static char* read_file_safe(const char *filename, size_t *out_size) {
    if (!filename || !out_size) return NULL;
    
    *out_size = 0;
    
    // Open file with O_NOFOLLOW to prevent symlink attacks
    int fd = open(filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open file\\n");
        return NULL;
    }
    
    // Validate the opened handle with fstat (TOCTOU mitigation)
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Cannot stat file\\n");
        close(fd);
        return NULL;
    }
    
    // Ensure it's a regular file\n    if (!S_ISREG(st.st_mode)) {\n        fprintf(stderr, "Error: Not a regular file\\n");\n        close(fd);\n        return NULL;\n    }\n    \n    // Check size limits\n    if (st.st_size <= 0 || st.st_size > MAX_FILE_SIZE) {\n        fprintf(stderr, "Error: File size out of bounds\\n");\n        close(fd);\n        return NULL;\n    }\n    \n    // Allocate buffer with size validation\n    size_t file_size = (size_t)st.st_size;\n    char *buffer = (char*)calloc(file_size + 1, 1);\n    if (!buffer) {\n        fprintf(stderr, "Error: Memory allocation failed\\n");\n        close(fd);\n        return NULL;\n    }\n    \n    // Read from the validated fd handle\n    ssize_t bytes_read = read(fd, buffer, file_size);\n    close(fd);\n    \n    if (bytes_read != (ssize_t)file_size) {\n        fprintf(stderr, "Error: Read failed\\n");\n        explicit_zero(buffer, file_size + 1);\n        free(buffer);\n        return NULL;\n    }\n    \n    buffer[file_size] = '\\0';\n    *out_size = file_size;\n    return buffer;\n}\n\n// Parse XML with XXE protection\nstatic xmlDocPtr parse_xml_safe(const char *xml_content, size_t xml_size) {\n    if (!xml_content || xml_size == 0) return NULL;\n    \n    // Disable external entity loading (XXE protection)\n    xmlSetExternalEntityLoader(NULL);\n    xmlSubstituteEntitiesDefault(0);\n    xmlLoadExtDtdDefaultValue = 0;\n    \n    // Parse with secure options: no network, no entity expansion\n    xmlDocPtr doc = xmlReadMemory(xml_content, (int)xml_size, NULL, NULL,\n                                  XML_PARSE_NONET | XML_PARSE_NOCDATA | \n                                  XML_PARSE_NOERROR | XML_PARSE_NOWARNING);\n    \n    return doc;\n}\n\n// Execute XPath query safely\nstatic char* execute_xpath(xmlDocPtr doc, const char *sanitized_id) {\n    if (!doc || !sanitized_id) return NULL;\n    \n    // Construct XPath query with sanitized ID\n    char xpath_query[MAX_XPATH_LEN];\n    int ret = snprintf(xpath_query, sizeof(xpath_query), "//*[@id='%s']", sanitized_id);\n    if (ret < 0 || ret >= (int)sizeof(xpath_query)) {\n        fprintf(stderr, "Error: XPath construction failed\\n");\n        return NULL;\n    }\n    \n    // Create XPath context\n    xmlXPathContextPtr xpath_ctx = xmlXPathNewContext(doc);\n    if (!xpath_ctx) {\n        fprintf(stderr, "Error: XPath context creation failed\\n");\n        return NULL;\n    }\n    \n    // Evaluate XPath expression\n    xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression((const xmlChar*)xpath_query, xpath_ctx);\n    xmlXPathFreeContext(xpath_ctx);\n    \n    if (!xpath_obj) {\n        fprintf(stderr, "Error: XPath evaluation failed\\n");\n        return NULL;\n    }\n    \n    char *result = NULL;\n    xmlNodeSetPtr nodes = xpath_obj->nodesetval;\n    \n    if (nodes && nodes->nodeNr > 0) {\n        // Get the first matching node's content
        xmlNodePtr node = nodes->nodeTab[0];
        xmlChar *content = xmlNodeGetContent(node);
        if (content) {
            size_t len = strlen((const char*)content);
            result = (char*)malloc(len + 1);
            if (result) {
                memcpy(result, content, len);
                result[len] = '\\0';
            }
            xmlFree(content);
        }
    }
    
    xmlXPathFreeObject(xpath_obj);
    return result;
}

int main(int argc, char *argv[]) {
    // Input validation
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <xpath_id> <xml_filename>\\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // Initialize libxml2
    xmlInitParser();
    LIBXML_TEST_VERSION
    
    char sanitized_id[MAX_ID_LEN];
    char *xml_content = NULL;
    xmlDocPtr doc = NULL;
    char *result = NULL;
    int exit_code = EXIT_FAILURE;
    
    // Sanitize the ID parameter (XPath injection prevention)
    if (sanitize_id(argv[1], sanitized_id, sizeof(sanitized_id)) != 0) {
        fprintf(stderr, "Error: Invalid ID format\\n");
        goto cleanup;
    }
    
    // Read XML file with race-safe validation
    size_t xml_size = 0;
    xml_content = read_file_safe(argv[2], &xml_size);
    if (!xml_content) {
        goto cleanup;
    }
    
    // Parse XML with XXE protection
    doc = parse_xml_safe(xml_content, xml_size);
    if (!doc) {
        fprintf(stderr, "Error: XML parsing failed\\n");
        goto cleanup;
    }
    
    // Execute XPath query
    result = execute_xpath(doc, sanitized_id);
    if (result) {
        printf("Result: %s\\n", result);
        exit_code = EXIT_SUCCESS;
    } else {
        printf("No matching element found\\n");
    }
    
cleanup:
    // Secure cleanup
    if (result) {
        explicit_zero(result, strlen(result));
        free(result);
    }
    if (doc) {
        xmlFreeDoc(doc);
    }
    if (xml_content) {
        explicit_zero(xml_content, xml_size);
        free(xml_content);
    }
    
    xmlCleanupParser();
    return exit_code;
}
