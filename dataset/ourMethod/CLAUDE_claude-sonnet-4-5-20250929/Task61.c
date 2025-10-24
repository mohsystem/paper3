
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE (1024 * 1024)  // 1MB limit for security
#define MAX_TAG_LENGTH 1024

// Secure function to clear sensitive data
static void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Skip whitespace characters safely with bounds checking
static size_t skip_whitespace(const char* xml, size_t len, size_t pos) {
    if (xml == NULL) return pos;
    while (pos < len && (xml[pos] == ' ' || xml[pos] == '\\t' || 
                         xml[pos] == '\\n' || xml[pos] == '\\r')) {
        pos++;
    }
    return pos;
}

// Validate tag name character
static bool is_valid_tag_char(char c) {
    return (c >= 'a' && c <= 'z') || 
           (c >= 'A' && c <= 'Z') || 
           (c >= '0' && c <= '9') || 
           c == '_' || c == '-' || c == ':' || c == '.';
}

// Safe substring search with bounds checking
static bool starts_with(const char* str, size_t str_len, size_t pos, const char* prefix) {
    if (str == NULL || prefix == NULL) return false;
    size_t prefix_len = strlen(prefix);
    if (pos + prefix_len > str_len) return false;
    return memcmp(str + pos, prefix, prefix_len) == 0;
}

// Safe find function with bounds checking
static size_t safe_find(const char* str, size_t str_len, size_t start_pos, const char* needle) {
    if (str == NULL || needle == NULL || start_pos >= str_len) {
        return str_len; // Return length as "not found" indicator
    }
    
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return start_pos;
    
    // Bounds check to prevent overflow
    if (str_len < needle_len) return str_len;
    
    for (size_t i = start_pos; i <= str_len - needle_len; i++) {
        if (memcmp(str + i, needle, needle_len) == 0) {
            return i;
        }
    }
    return str_len; // Not found
}

// Extract root element from XML string
// Returns allocated string (caller must free) or NULL on error
char* get_root_element(const char* xml, char** error_msg) {
    // Initialize error message pointer
    if (error_msg != NULL) {
        *error_msg = NULL;
    }
    
    // Input validation: NULL check
    if (xml == NULL) {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid input: NULL XML string");
        }
        return NULL;
    }
    
    // Input validation: length check
    size_t xml_len = strlen(xml);
    if (xml_len == 0 || xml_len > MAX_INPUT_SIZE) {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid input: XML string empty or exceeds size limit");
        }
        return NULL;
    }
    
    size_t pos = 0;
    
    // Skip leading whitespace
    pos = skip_whitespace(xml, xml_len, pos);
    if (pos >= xml_len) {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid XML: no content after whitespace");
        }
        return NULL;
    }
    
    // Skip XML declaration if present (<?xml ... ?>)
    if (starts_with(xml, xml_len, pos, "<?xml")) {
        pos += 5;
        size_t end_pos = safe_find(xml, xml_len, pos, "?>");
        if (end_pos >= xml_len) {
            if (error_msg != NULL) {
                *error_msg = strdup("Invalid XML: unclosed declaration");
            }
            return NULL;
        }
        pos = end_pos + 2;
        pos = skip_whitespace(xml, xml_len, pos);
    }
    
    // Skip DOCTYPE but reject external entities (security measure)
    if (starts_with(xml, xml_len, pos, "<!DOCTYPE")) {
        size_t end_pos = safe_find(xml, xml_len, pos, ">");
        if (end_pos >= xml_len) {
            if (error_msg != NULL) {
                *error_msg = strdup("Invalid XML: unclosed DOCTYPE");
            }
            return NULL;
        }
        
        // Security check: look for external entity references
        size_t doctype_len = end_pos - pos;
        if (doctype_len < xml_len) {
            // Use safe bounded search for SYSTEM/PUBLIC
            for (size_t i = pos; i < end_pos - 6; i++) {
                if (memcmp(xml + i, "SYSTEM", 6) == 0 || memcmp(xml + i, "PUBLIC", 6) == 0) {
                    if (error_msg != NULL) {
                        *error_msg = strdup("Security: external entities not allowed");
                    }
                    return NULL;
                }
            }
        }
        
        pos = end_pos + 1;
        pos = skip_whitespace(xml, xml_len, pos);
    }
    
    // Skip comments (<!-- ... -->)
    while (starts_with(xml, xml_len, pos, "<!--")) {
        size_t end_pos = safe_find(xml, xml_len, pos + 4, "-->");
        if (end_pos >= xml_len) {
            if (error_msg != NULL) {
                *error_msg = strdup("Invalid XML: unclosed comment");
            }
            return NULL;
        }
        pos = end_pos + 3;
        pos = skip_whitespace(xml, xml_len, pos);
    }
    
    // Now expect root element opening tag
    if (pos >= xml_len || xml[pos] != '<') {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid XML: expected opening tag");
        }
        return NULL;
    }
    pos++;
    
    // Check for processing instruction or other special tags
    if (pos < xml_len && (xml[pos] == '?' || xml[pos] == '!')) {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid XML: unexpected special tag where root expected");
        }
        return NULL;
    }
    
    // Extract tag name with bounds checking
    size_t tag_start = pos;
    
    // First character must be letter or underscore
    if (pos >= xml_len || 
        !((xml[pos] >= 'a' && xml[pos] <= 'z') || 
          (xml[pos] >= 'A' && xml[pos] <= 'Z') || 
          xml[pos] == '_')) {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid XML: tag name must start with letter or underscore");
        }
        return NULL;
    }
    
    // Find end of tag name
    size_t tag_length = 0;
    while (pos < xml_len && tag_length < MAX_TAG_LENGTH) {
        char c = xml[pos];
        
        // Tag name ends at whitespace, '>', '/', or invalid character
        if (c == ' ' || c == '\\t' || c == '\\n' || c == '\\r' || 
            c == '>' || c == '/') {
            break;
        }
        
        // Validate character
        if (!is_valid_tag_char(c)) {
            if (error_msg != NULL) {
                *error_msg = strdup("Invalid XML: illegal character in tag name");
            }
            return NULL;
        }
        
        pos++;
        tag_length++;
    }
    
    // Validate tag length
    if (tag_length == 0) {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid XML: empty tag name");
        }
        return NULL;
    }
    if (tag_length >= MAX_TAG_LENGTH) {
        if (error_msg != NULL) {
            *error_msg = strdup("Invalid XML: tag name too long");
        }
        return NULL;
    }
    
    // Allocate memory for root element name with bounds check
    char* root_element = (char*)malloc(tag_length + 1);
    if (root_element == NULL) {
        if (error_msg != NULL) {
            *error_msg = strdup("Memory allocation failed");
        }
        return NULL;
    }
    
    // Copy tag name safely with explicit null termination
    memcpy(root_element, xml + tag_start, tag_length);
    root_element[tag_length] = '\\0';
    
    return root_element;
}

// Main function with test cases
int main(void) {
    char* root = NULL;
    char* error = NULL;
    
    // Test case 1: Simple XML
    printf("Test 1: Simple XML\\n");
    root = get_root_element("<root><child>value</child></root>", &error);
    if (root != NULL) {
        printf("Root element: %s\\n", root);
        secure_zero(root, strlen(root));
        free(root);
        root = NULL;
    } else if (error != NULL) {
        printf("Error: %s\\n", error);
        free(error);
        error = NULL;
    }
    printf("\\n");
    
    // Test case 2: XML with declaration
    printf("Test 2: XML with declaration\\n");
    root = get_root_element("<?xml version=\\"1.0\\" encoding=\\"UTF-8\\"?><document><section>text</section></document>", &error);
    if (root != NULL) {
        printf("Root element: %s\\n", root);
        secure_zero(root, strlen(root));
        free(root);
        root = NULL;
    } else if (error != NULL) {
        printf("Error: %s\\n", error);
        free(error);
        error = NULL;
    }
    printf("\\n");
    
    // Test case 3: XML with whitespace and comments
    printf("Test 3: XML with whitespace and comments\\n");
    root = get_root_element("  \\n  <!-- Comment -->\\n  <root-element><child/></root-element>", &error);
    if (root != NULL) {
        printf("Root element: %s\\n", root);
        secure_zero(root, strlen(root));
        free(root);
        root = NULL;
    } else if (error != NULL) {
        printf("Error: %s\\n", error);
        free(error);
        error = NULL;
    }
    printf("\\n");
    
    // Test case 4: XML with namespace
    printf("Test 4: XML with namespace\\n");
    root = get_root_element("<ns:root xmlns:ns=\\"http://example.com\\"><ns:child/></ns:root>", &error);
    if (root != NULL) {
        printf("Root element: %s\\n", root);
        secure_zero(root, strlen(root));
        free(root);
        root = NULL;
    } else if (error != NULL) {
        printf("Error: %s\\n", error);
        free(error);
        error = NULL;
    }
    printf("\\n");
    
    // Test case 5: Invalid XML with external entity (should fail for security)
    printf("Test 5: Invalid XML with external entity\\n");
    root = get_root_element("<!DOCTYPE root SYSTEM \\"external.dtd\\"><root/>", &error);
    if (root != NULL) {
        printf("Root element: %s\\n", root);
        secure_zero(root, strlen(root));
        free(root);
        root = NULL;
    } else if (error != NULL) {
        printf("Error (expected): %s\\n", error);
        free(error);
        error = NULL;
    }
    printf("\\n");
    
    return 0;
}
