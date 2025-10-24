
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Maximum nesting depth to prevent stack overflow
#define MAX_DEPTH 1000
// Maximum string value size to prevent excessive memory usage
#define MAX_STRING_SIZE 1000000
// Maximum JSON document size
#define MAX_JSON_SIZE 100000000

// JSON Node types
typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL
} JsonType;

// JSON Node structure
typedef struct {
    JsonType type;
    char* value;  // Dynamically allocated string for the value
} JsonNode;

// Parser context structure
typedef struct {
    const char* json;
    size_t pos;
    size_t length;
    size_t current_depth;
    char error_msg[256];  // Fixed-size buffer for error messages
} JsonParser;

// Initialize JsonNode - returns NULL on allocation failure
static JsonNode* json_node_create(JsonType type, const char* value) {
    // Allocate memory for node with NULL check
    JsonNode* node = (JsonNode*)malloc(sizeof(JsonNode));
    if (node == NULL) {
        return NULL;
    }
    
    node->type = type;
    node->value = NULL;
    
    // Allocate and copy value if provided
    if (value != NULL) {
        size_t len = strlen(value);
        // Bounds check: prevent excessive allocation
        if (len > MAX_STRING_SIZE) {
            free(node);
            return NULL;
        }
        
        node->value = (char*)malloc(len + 1);
        if (node->value == NULL) {
            free(node);
            return NULL;
        }
        // Use strncpy with explicit null termination for safety
        strncpy(node->value, value, len);
        node->value[len] = '\\0';
    }
    
    return node;
}

// Free JsonNode and its value
static void json_node_free(JsonNode* node) {
    if (node == NULL) {
        return;
    }
    
    // Free value string if allocated
    if (node->value != NULL) {
        // Clear sensitive data before freeing (defense in depth)
        memset(node->value, 0, strlen(node->value));
        free(node->value);
        node->value = NULL;
    }
    
    free(node);
}

// Get string representation of JsonType
static const char* json_type_to_string(JsonType type) {
    switch (type) {
        case JSON_OBJECT: return "OBJECT";
        case JSON_ARRAY: return "ARRAY";
        case JSON_STRING: return "STRING";
        case JSON_NUMBER: return "NUMBER";
        case JSON_BOOLEAN: return "BOOLEAN";
        case JSON_NULL: return "NULL";
        default: return "UNKNOWN";
    }
}

// Set error message with bounds checking
static void set_error(JsonParser* parser, const char* msg) {
    if (parser == NULL || msg == NULL) {
        return;
    }
    // Use snprintf to prevent buffer overflow
    snprintf(parser->error_msg, sizeof(parser->error_msg), "%s", msg);
}

// Skip whitespace with bounds checking
static void skip_whitespace(JsonParser* parser) {
    if (parser == NULL) {
        return;
    }
    
    // Bounds check: ensure pos is within valid range
    while (parser->pos < parser->length) {
        char c = parser->json[parser->pos];
        if (c == ' ' || c == '\\t' || c == '\\n' || c == '\\r') {
            parser->pos++;
        } else {
            break;
        }
    }
}

// Parse string value with proper validation - returns NULL on error
static char* parse_string(JsonParser* parser) {
    if (parser == NULL) {
        return NULL;
    }
    
    // Input validation: ensure we start with a quote
    if (parser->pos >= parser->length || parser->json[parser->pos] != '"') {
        set_error(parser, "Expected opening quote for string");
        return NULL;
    }
    parser->pos++; // Skip opening quote
    
    // Allocate initial buffer with size check
    size_t capacity = 64;
    size_t length = 0;
    char* result = (char*)malloc(capacity);
    if (result == NULL) {
        set_error(parser, "Memory allocation failed");
        return NULL;
    }
    
    // Bounds check: ensure we don't read past end of input\n    while (parser->pos < parser->length && parser->json[parser->pos] != '"') {\n        // Prevent excessive memory usage\n        if (length >= MAX_STRING_SIZE) {\n            set_error(parser, "String value exceeds maximum allowed size");\n            free(result);\n            return NULL;\n        }\n        \n        // Grow buffer if needed with overflow check\n        if (length + 2 >= capacity) {\n            // Check for overflow before doubling\n            if (capacity > MAX_STRING_SIZE / 2) {\n                set_error(parser, "String buffer size exceeds limit");\n                free(result);\n                return NULL;\n            }\n            size_t new_capacity = capacity * 2;\n            char* new_result = (char*)realloc(result, new_capacity);\n            if (new_result == NULL) {\n                set_error(parser, "Memory reallocation failed");\n                free(result);\n                return NULL;\n            }\n            result = new_result;\n            capacity = new_capacity;\n        }\n        \n        if (parser->json[parser->pos] == '\\\\') {\n            parser->pos++;\n            // Bounds check after escape character\n            if (parser->pos >= parser->length) {\n                set_error(parser, "Unterminated escape sequence");\n                free(result);\n                return NULL;\n            }\n            \n            // Handle escape sequences with validation\n            switch (parser->json[parser->pos]) {\n                case '"': result[length++] = '"'; break;\n                case '\\\\': result[length++] = '\\\\'; break;\n                case '/': result[length++] = '/'; break;\n                case 'b': result[length++] = '\\b'; break;\n                case 'f': result[length++] = '\\f'; break;\n                case 'n': result[length++] = '\\n'; break;\n                case 'r': result[length++] = '\\r'; break;\n                case 't': result[length++] = '\\t'; break;\n                case 'u':\n                    // Bounds check: ensure we have 4 hex digits\n                    if (parser->pos + 4 >= parser->length) {\n                        set_error(parser, "Invalid unicode escape sequence");\n                        free(result);\n                        return NULL;\n                    }\n                    // Validate hex digits\n                    for (int i = 1; i <= 4; i++) {\n                        char c = parser->json[parser->pos + i];\n                        if (!isxdigit((unsigned char)c)) {\n                            set_error(parser, "Invalid unicode escape sequence");\n                            free(result);\n                            return NULL;\n                        }\n                    }\n                    parser->pos += 4;\n                    result[length++] = 'U'; // Placeholder\n                    break;\n                default:\n                    set_error(parser, "Invalid escape sequence");\n                    free(result);\n                    return NULL;\n            }\n        } else {\n            result[length++] = parser->json[parser->pos];\n        }\n        parser->pos++;\n    }\n    \n    // Validation: ensure string was properly terminated\n    if (parser->pos >= parser->length || parser->json[parser->pos] != '"') {\n        set_error(parser, "Unterminated string");\n        free(result);\n        return NULL;\n    }\n    parser->pos++; // Skip closing quote\n    \n    // Null-terminate the string\n    result[length] = '\\0';\n    \n    return result;\n}\n\n// Parse number with validation - returns NULL on error\nstatic char* parse_number(JsonParser* parser) {\n    if (parser == NULL) {\n        return NULL;\n    }\n    \n    size_t start = parser->pos;\n    \n    // Bounds check and validation: optional minus\n    if (parser->pos < parser->length && parser->json[parser->pos] == '-') {\n        parser->pos++;\n    }\n    \n    // Validation: at least one digit required\n    if (parser->pos >= parser->length || !isdigit((unsigned char)parser->json[parser->pos])) {\n        set_error(parser, "Invalid number format");\n        return NULL;\n    }\n    \n    // Parse integer part with bounds checking\n    if (parser->json[parser->pos] == '0') {\n        parser->pos++;\n    } else {\n        while (parser->pos < parser->length && isdigit((unsigned char)parser->json[parser->pos])) {\n            parser->pos++;\n        }\n    }\n    \n    // Parse fractional part if present\n    if (parser->pos < parser->length && parser->json[parser->pos] == '.') {\n        parser->pos++;\n        // Validation: at least one digit required after decimal point\n        if (parser->pos >= parser->length || !isdigit((unsigned char)parser->json[parser->pos])) {\n            set_error(parser, "Invalid number format: expected digit after decimal point");\n            return NULL;\n        }\n        while (parser->pos < parser->length && isdigit((unsigned char)parser->json[parser->pos])) {\n            parser->pos++;\n        }\n    }\n    \n    // Parse exponent if present\n    if (parser->pos < parser->length && (parser->json[parser->pos] == 'e' || parser->json[parser->pos] == 'E')) {\n        parser->pos++;\n        // Optional sign with bounds check\n        if (parser->pos < parser->length && (parser->json[parser->pos] == '+' || parser->json[parser->pos] == '-')) {\n            parser->pos++;\n        }\n        // Validation: at least one digit required in exponent\n        if (parser->pos >= parser->length || !isdigit((unsigned char)parser->json[parser->pos])) {\n            set_error(parser, "Invalid number format: expected digit in exponent");\n            return NULL;\n        }\n        while (parser->pos < parser->length && isdigit((unsigned char)parser->json[parser->pos])) {\n            parser->pos++;\n        }\n    }\n    \n    // Calculate length and allocate with bounds check\n    size_t num_len = parser->pos - start;\n    if (num_len > MAX_STRING_SIZE) {\n        set_error(parser, "Number string too long");\n        return NULL;\n    }\n    \n    char* result = (char*)malloc(num_len + 1);\n    if (result == NULL) {\n        set_error(parser, "Memory allocation failed");\n        return NULL;\n    }\n    \n    // Copy number string with bounds check\n    memcpy(result, parser->json + start, num_len);\n    result[num_len] = '\\0';\n    \n    return result;\n}\n\n// Forward declarations for recursive parsing\nstatic JsonNode* parse_value(JsonParser* parser);\n\n// Parse JSON object with depth limiting\nstatic JsonNode* parse_object(JsonParser* parser) {\n    if (parser == NULL) {\n        return NULL;\n    }\n    \n    // Depth check: prevent stack overflow from deeply nested structures\n    if (parser->current_depth >= MAX_DEPTH) {\n        set_error(parser, "Maximum nesting depth exceeded");\n        return NULL;\n    }\n    parser->current_depth++;\n    \n    // Bounds check and validation: ensure opening brace\n    if (parser->pos >= parser->length || parser->json[parser->pos] != '{') {\n        set_error(parser, "Expected opening brace for object");\n        parser->current_depth--;\n        return NULL;\n    }\n    parser->pos++;\n    \n    skip_whitespace(parser);\n    \n    // Handle empty object\n    if (parser->pos < parser->length && parser->json[parser->pos] == '}') {\n        parser->pos++;\n        parser->current_depth--;\n        return json_node_create(JSON_OBJECT, "{}");\n    }\n    \n    // Parse key-value pairs with validation\n    while (parser->pos < parser->length) {\n        skip_whitespace(parser);\n        \n        // Validation: key must be a string\n        if (parser->pos >= parser->length || parser->json[parser->pos] != '"') {\n            set_error(parser, "Expected string key in object");\n            parser->current_depth--;\n            return NULL;\n        }\n        \n        char* key = parse_string(parser);\n        if (key == NULL) {\n            parser->current_depth--;\n            return NULL;\n        }\n        free(key); // Free key as we only need to validate structure\n        \n        skip_whitespace(parser);\n        \n        // Validation: expect colon after key\n        if (parser->pos >= parser->length || parser->json[parser->pos] != ':') {\n            set_error(parser, "Expected colon after object key");\n            parser->current_depth--;\n            return NULL;\n        }\n        parser->pos++;\n        \n        skip_whitespace(parser);\n        \n        // Parse value recursively\n        JsonNode* value = parse_value(parser);\n        if (value == NULL) {\n            parser->current_depth--;\n            return NULL;\n        }\n        json_node_free(value); // Free value as we only need to validate structure\n        \n        skip_whitespace(parser);\n        \n        // Check for continuation or end\n        if (parser->pos >= parser->length) {\n            set_error(parser, "Unexpected end of input in object");\n            parser->current_depth--;\n            return NULL;\n        }\n        \n        if (parser->json[parser->pos] == ',') {\n            parser->pos++;\n            skip_whitespace(parser);\n            continue;\n        } else if (parser->json[parser->pos] == '}') {\n            parser->pos++;\n            parser->current_depth--;\n            return json_node_create(JSON_OBJECT, "{...}");\n        } else {\n            set_error(parser, "Expected comma or closing brace in object");\n            parser->current_depth--;\n            return NULL;\n        }\n    }\n    \n    set_error(parser, "Unterminated object");\n    parser->current_depth--;\n    return NULL;\n}\n\n// Parse JSON array with depth limiting\nstatic JsonNode* parse_array(JsonParser* parser) {\n    if (parser == NULL) {\n        return NULL;\n    }\n    \n    // Depth check: prevent stack overflow\n    if (parser->current_depth >= MAX_DEPTH) {\n        set_error(parser, "Maximum nesting depth exceeded");\n        return NULL;\n    }\n    parser->current_depth++;\n    \n    // Bounds check and validation: ensure opening bracket\n    if (parser->pos >= parser->length || parser->json[parser->pos] != '[') {\n        set_error(parser, "Expected opening bracket for array");\n        parser->current_depth--;\n        return NULL;\n    }\n    parser->pos++;\n    \n    skip_whitespace(parser);\n    \n    // Handle empty array\n    if (parser->pos < parser->length && parser->json[parser->pos] == ']') {\n        parser->pos++;\n        parser->current_depth--;\n        return json_node_create(JSON_ARRAY, "[]");\n    }\n    \n    // Parse array elements with validation\n    while (parser->pos < parser->length) {\n        skip_whitespace(parser);\n        \n        // Parse element recursively\n        JsonNode* element = parse_value(parser);\n        if (element == NULL) {\n            parser->current_depth--;\n            return NULL;\n        }\n        json_node_free(element); // Free element as we only need to validate structure\n        \n        skip_whitespace(parser);\n        \n        // Check for continuation or end\n        if (parser->pos >= parser->length) {\n            set_error(parser, "Unexpected end of input in array");\n            parser->current_depth--;\n            return NULL;\n        }\n        \n        if (parser->json[parser->pos] == ',') {\n            parser->pos++;\n            skip_whitespace(parser);\n            continue;\n        } else if (parser->json[parser->pos] == ']') {\n            parser->pos++;\n            parser->current_depth--;\n            return json_node_create(JSON_ARRAY, "[...]");\n        } else {\n            set_error(parser, "Expected comma or closing bracket in array");\n            parser->current_depth--;\n            return NULL;\n        }\n    }\n    \n    set_error(parser, "Unterminated array");\n    parser->current_depth--;\n    return NULL;\n}\n\n// Parse keyword (true, false, null) with validation\nstatic JsonNode* parse_keyword(JsonParser* parser) {\n    if (parser == NULL) {\n        return NULL;\n    }\n    \n    // Bounds check and validation for "true"\n    if (parser->pos + 4 <= parser->length && strncmp(parser->json + parser->pos, "true", 4) == 0) {\n        parser->pos += 4;\n        return json_node_create(JSON_BOOLEAN, "true");\n    }\n    \n    // Bounds check and validation for "false"\n    if (parser->pos + 5 <= parser->length && strncmp(parser->json + parser->pos, "false", 5) == 0) {\n        parser->pos += 5;\n        return json_node_create(JSON_BOOLEAN, "false");\n    }\n    \n    // Bounds check and validation for "null"\n    if (parser->pos + 4 <= parser->length && strncmp(parser->json + parser->pos, "null", 4) == 0) {\n        parser->pos += 4;\n        return json_node_create(JSON_NULL, "null");\n    }\n    \n    set_error(parser, "Invalid keyword");\n    return NULL;\n}\n\n// Parse any JSON value with validation\nstatic JsonNode* parse_value(JsonParser* parser) {\n    if (parser == NULL) {\n        return NULL;\n    }\n    \n    skip_whitespace(parser);\n    \n    // Bounds check: ensure we have data to parse\n    if (parser->pos >= parser->length) {\n        set_error(parser, "Unexpected end of input");\n        return NULL;\n    }\n    \n    // Determine value type and parse accordingly\n    char c = parser->json[parser->pos];\n    \n    if (c == '{') {\n        return parse_object(parser);\n    } else if (c == '[') {\n        return parse_array(parser);\n    } else if (c == '"') {\n        char* str_val = parse_string(parser);\n        if (str_val == NULL) {\n            return NULL;\n        }\n        JsonNode* node = json_node_create(JSON_STRING, str_val);\n        free(str_val);\n        return node;\n    } else if (c == '-' || isdigit((unsigned char)c)) {\n        char* num_val = parse_number(parser);\n        if (num_val == NULL) {\n            return NULL;\n        }\n        JsonNode* node = json_node_create(JSON_NUMBER, num_val);\n        free(num_val);\n        return node;\n    } else if (c == 't' || c == 'f' || c == 'n') {\n        return parse_keyword(parser);\n    } else {\n        set_error(parser, "Invalid JSON value");\n        return NULL;\n    }\n}\n\n// Initialize parser with input validation\nstatic bool parser_init(JsonParser* parser, const char* json_str) {\n    if (parser == NULL || json_str == NULL) {\n        return false;\n    }\n    \n    // Initialize all fields\n    parser->json = json_str;\n    parser->pos = 0;\n    parser->length = strlen(json_str);\n    parser->current_depth = 0;\n    parser->error_msg[0] = '\\0';
    
    // Input validation: check for empty input
    if (parser->length == 0) {
        set_error(parser, "Empty JSON input");
        return false;
    }
    
    // Input validation: check for reasonable size to prevent DoS
    if (parser->length > MAX_JSON_SIZE) {
        set_error(parser, "JSON input exceeds maximum allowed size");
        return false;
    }
    
    return true;
}

// Parse JSON string and return root element
JsonNode* parse_json_root(const char* json_string) {
    // Input validation: check for NULL pointer
    if (json_string == NULL) {
        return NULL;
    }
    
    // Initialize parser with validation
    JsonParser parser;
    if (!parser_init(&parser, json_string)) {
        return NULL;
    }
    
    skip_whitespace(&parser);
    
    // Validation: ensure we have content to parse
    if (parser.pos >= parser.length) {
        return NULL;
    }
    
    // Parse the root value
    JsonNode* root = parse_value(&parser);
    if (root == NULL) {
        return NULL;
    }
    
    skip_whitespace(&parser);
    
    // Validation: ensure no trailing content
    if (parser.pos < parser.length) {
        json_node_free(root);
        return NULL;
    }
    
    return root;
}

int main(void) {
    // Test case 1: Simple object
    {
        const char* json1 = "{\\"name\\": \\"John\\", \\"age\\": 30}";
        JsonNode* root1 = parse_json_root(json1);
        if (root1 != NULL) {
            printf("Test 1 - Root type: %s\\n", json_type_to_string(root1->type));
            json_node_free(root1);
        } else {
            printf("Test 1 failed\\n");
        }
    }
    
    // Test case 2: Array root
    {
        const char* json2 = "[1, 2, 3, \\"test\\"]";
        JsonNode* root2 = parse_json_root(json2);
        if (root2 != NULL) {
            printf("Test 2 - Root type: %s\\n", json_type_to_string(root2->type));
            json_node_free(root2);
        } else {
            printf("Test 2 failed\\n");
        }
    }
    
    // Test case 3: Nested structure
    {
        const char* json3 = "{\\"data\\": {\\"items\\": [1, 2, 3]}, \\"status\\": true}";
        JsonNode* root3 = parse_json_root(json3);
        if (root3 != NULL) {
            printf("Test 3 - Root type: %s\\n", json_type_to_string(root3->type));
            json_node_free(root3);
        } else {
            printf("Test 3 failed\\n");
        }
    }
    
    // Test case 4: String root
    {
        const char* json4 = "\\"hello world\\"";
        JsonNode* root4 = parse_json_root(json4);
        if (root4 != NULL) {
            printf("Test 4 - Root type: %s, Value: %s\\n", 
                   json_type_to_string(root4->type), 
                   root4->value ? root4->value : "NULL");
            json_node_free(root4);
        } else {
            printf("Test 4 failed\\n");
        }
    }
    
    // Test case 5: Number root
    {
        const char* json5 = "42.5";
        JsonNode* root5 = parse_json_root(json5);
        if (root5 != NULL) {
            printf("Test 5 - Root type: %s, Value: %s\\n", 
                   json_type_to_string(root5->type), 
                   root5->value ? root5->value : "NULL");
            json_node_free(root5);
        } else {
            printf("Test 5 failed\\n");
        }
    }
    
    return 0;
}
