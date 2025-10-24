
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <limits>

// JSON Node types
enum class JsonType {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER,
    BOOLEAN,
    NULL_TYPE
};

// JSON Node representation
class JsonNode {
public:
    JsonType type;
    std::string value;
    
    explicit JsonNode(JsonType t) : type(t), value("") {}
    JsonNode(JsonType t, const std::string& v) : type(t), value(v) {}
    
    std::string getTypeString() const {
        switch(type) {
            case JsonType::OBJECT: return "OBJECT";
            case JsonType::ARRAY: return "ARRAY";
            case JsonType::STRING: return "STRING";
            case JsonType::NUMBER: return "NUMBER";
            case JsonType::BOOLEAN: return "BOOLEAN";
            case JsonType::NULL_TYPE: return "NULL";
            default: return "UNKNOWN";
        }
    }
};

class JsonParser {
private:
    const char* json;
    size_t pos;
    size_t length;
    static constexpr size_t MAX_DEPTH = 1000; // Prevent stack overflow from deeply nested JSON
    size_t current_depth;
    
    // Skip whitespace characters
    void skipWhitespace() {
        // Bounds check: ensure pos is within valid range
        while (pos < length && (json[pos] == ' ' || json[pos] == '\\t' || 
               json[pos] == '\\n' || json[pos] == '\\r')) {
            pos++;
        }
    }
    
    // Parse string value with proper validation
    std::string parseString() {
        // Input validation: ensure we start with a quote
        if (pos >= length || json[pos] != '"') {
            throw std::runtime_error("Expected opening quote for string");
        }
        pos++; // Skip opening quote
        
        std::string result;
        result.reserve(64); // Pre-allocate to reduce reallocations
        
        // Bounds check: ensure we don't read past end of input\n        while (pos < length && json[pos] != '"') {\n            // Prevent excessive memory usage\n            if (result.size() > 1000000) { // 1MB limit for single string\n                throw std::runtime_error("String value exceeds maximum allowed size");\n            }\n            \n            if (json[pos] == '\\\\') {\n                pos++;\n                // Bounds check after escape character\n                if (pos >= length) {\n                    throw std::runtime_error("Unterminated escape sequence");\n                }\n                // Handle escape sequences with validation\n                switch (json[pos]) {\n                    case '"': result += '"'; break;\n                    case '\\\\': result += '\\\\'; break;\n                    case '/': result += '/'; break;\n                    case 'b': result += '\\b'; break;\n                    case 'f': result += '\\f'; break;\n                    case 'n': result += '\\n'; break;\n                    case 'r': result += '\\r'; break;\n                    case 't': result += '\\t'; break;\n                    case 'u': // Unicode escape - validate but simplify for this implementation\n                        // Bounds check: ensure we have 4 hex digits\n                        if (pos + 4 >= length) {\n                            throw std::runtime_error("Invalid unicode escape sequence");\n                        }\n                        // Skip unicode handling for simplicity but validate format\n                        for (int i = 1; i <= 4; i++) {\n                            char c = json[pos + i];\n                            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {\n                                throw std::runtime_error("Invalid unicode escape sequence");\n                            }\n                        }\n                        pos += 4;\n                        result += 'U'; // Placeholder\n                        break;\n                    default:\n                        throw std::runtime_error("Invalid escape sequence");\n                }\n            } else {\n                result += json[pos];\n            }\n            pos++;\n        }\n        \n        // Validation: ensure string was properly terminated\n        if (pos >= length || json[pos] != '"') {\n            throw std::runtime_error("Unterminated string");\n        }\n        pos++; // Skip closing quote\n        \n        return result;\n    }\n    \n    // Parse number with validation\n    std::string parseNumber() {\n        size_t start = pos;\n        \n        // Bounds check and validation: optional minus\n        if (pos < length && json[pos] == '-') {\n            pos++;\n        }\n        \n        // Validation: at least one digit required\n        if (pos >= length || (json[pos] < '0' || json[pos] > '9')) {\n            throw std::runtime_error("Invalid number format");\n        }\n        \n        // Parse integer part with bounds checking\n        if (json[pos] == '0') {\n            pos++;\n        } else {\n            while (pos < length && json[pos] >= '0' && json[pos] <= '9') {\n                pos++;\n            }\n        }\n        \n        // Parse fractional part if present\n        if (pos < length && json[pos] == '.') {\n            pos++;\n            // Validation: at least one digit required after decimal point\n            if (pos >= length || json[pos] < '0' || json[pos] > '9') {\n                throw std::runtime_error("Invalid number format: expected digit after decimal point");\n            }\n            while (pos < length && json[pos] >= '0' && json[pos] <= '9') {\n                pos++;\n            }\n        }\n        \n        // Parse exponent if present\n        if (pos < length && (json[pos] == 'e' || json[pos] == 'E')) {\n            pos++;\n            // Optional sign with bounds check\n            if (pos < length && (json[pos] == '+' || json[pos] == '-')) {\n                pos++;\n            }\n            // Validation: at least one digit required in exponent\n            if (pos >= length || json[pos] < '0' || json[pos] > '9') {\n                throw std::runtime_error("Invalid number format: expected digit in exponent");\n            }\n            while (pos < length && json[pos] >= '0' && json[pos] <= '9') {\n                pos++;\n            }\n        }\n        \n        // Bounds check: ensure we didn't exceed bounds
        if (pos > length) {
            throw std::runtime_error("Number parsing exceeded input bounds");
        }
        
        return std::string(json + start, pos - start);
    }
    
    // Parse boolean or null with validation
    JsonNode parseKeyword() {
        // Bounds check and validation for "true"
        if (pos + 4 <= length && std::strncmp(json + pos, "true", 4) == 0) {
            pos += 4;
            return JsonNode(JsonType::BOOLEAN, "true");
        }
        
        // Bounds check and validation for "false"
        if (pos + 5 <= length && std::strncmp(json + pos, "false", 5) == 0) {
            pos += 5;
            return JsonNode(JsonType::BOOLEAN, "false");
        }
        
        // Bounds check and validation for "null"
        if (pos + 4 <= length && std::strncmp(json + pos, "null", 4) == 0) {
            pos += 4;
            return JsonNode(JsonType::NULL_TYPE, "null");
        }
        
        throw std::runtime_error("Invalid keyword");
    }
    
    // Forward declaration for recursive parsing
    JsonNode parseValue();
    
    // Parse JSON object with depth limiting
    JsonNode parseObject() {
        // Depth check: prevent stack overflow from deeply nested structures
        if (current_depth >= MAX_DEPTH) {
            throw std::runtime_error("Maximum nesting depth exceeded");
        }
        current_depth++;
        
        // Bounds check and validation: ensure opening brace
        if (pos >= length || json[pos] != '{') {
            throw std::runtime_error("Expected opening brace for object");
        }
        pos++;
        
        skipWhitespace();
        
        // Handle empty object
        if (pos < length && json[pos] == '}') {
            pos++;
            current_depth--;
            return JsonNode(JsonType::OBJECT, "{}");
        }
        
        // Parse key-value pairs with validation
        while (pos < length) {
            skipWhitespace();
            
            // Validation: key must be a string
            if (pos >= length || json[pos] != '"') {
                throw std::runtime_error("Expected string key in object");
            }
            parseString(); // Parse and discard key for root identification
            
            skipWhitespace();
            
            // Validation: expect colon after key
            if (pos >= length || json[pos] != ':') {
                throw std::runtime_error("Expected colon after object key");
            }
            pos++;
            
            skipWhitespace();
            
            // Parse value recursively
            parseValue();
            
            skipWhitespace();
            
            // Check for continuation or end
            if (pos >= length) {
                throw std::runtime_error("Unexpected end of input in object");
            }
            
            if (json[pos] == ',') {
                pos++;
                skipWhitespace();
                continue;
            } else if (json[pos] == '}') {
                pos++;
                current_depth--;
                return JsonNode(JsonType::OBJECT, "{...}");
            } else {
                throw std::runtime_error("Expected comma or closing brace in object");
            }
        }
        
        throw std::runtime_error("Unterminated object");
    }
    
    // Parse JSON array with depth limiting
    JsonNode parseArray() {
        // Depth check: prevent stack overflow
        if (current_depth >= MAX_DEPTH) {
            throw std::runtime_error("Maximum nesting depth exceeded");
        }
        current_depth++;
        
        // Bounds check and validation: ensure opening bracket
        if (pos >= length || json[pos] != '[') {
            throw std::runtime_error("Expected opening bracket for array");
        }
        pos++;
        
        skipWhitespace();
        
        // Handle empty array
        if (pos < length && json[pos] == ']') {
            pos++;
            current_depth--;
            return JsonNode(JsonType::ARRAY, "[]");
        }
        
        // Parse array elements with validation
        while (pos < length) {
            skipWhitespace();
            
            // Parse element recursively
            parseValue();
            
            skipWhitespace();
            
            // Check for continuation or end
            if (pos >= length) {
                throw std::runtime_error("Unexpected end of input in array");
            }
            
            if (json[pos] == ',') {
                pos++;
                skipWhitespace();
                continue;
            } else if (json[pos] == ']') {
                pos++;
                current_depth--;
                return JsonNode(JsonType::ARRAY, "[...]");
            } else {
                throw std::runtime_error("Expected comma or closing bracket in array");
            }
        }
        
        throw std::runtime_error("Unterminated array");
    }
    
    // Parse any JSON value with validation
    JsonNode parseValue() {
        skipWhitespace();
        
        // Bounds check: ensure we have data to parse
        if (pos >= length) {
            throw std::runtime_error("Unexpected end of input");
        }
        
        // Determine value type and parse accordingly
        char c = json[pos];
        
        if (c == '{') {
            return parseObject();
        } else if (c == '[') {
            return parseArray();
        } else if (c == '"') {
            std::string str = parseString();
            return JsonNode(JsonType::STRING, str);
        } else if (c == '-' || (c >= '0' && c <= '9')) {
            std::string num = parseNumber();
            return JsonNode(JsonType::NUMBER, num);
        } else if (c == 't' || c == 'f' || c == 'n') {
            return parseKeyword();
        } else {
            throw std::runtime_error("Invalid JSON value");
        }
    }
    
public:
    // Constructor with input validation
    explicit JsonParser(const std::string& jsonStr) : json(nullptr), pos(0), length(0), current_depth(0) {
        // Input validation: check for null or empty input
        if (jsonStr.empty()) {
            throw std::runtime_error("Empty JSON input");
        }
        
        // Input validation: check for reasonable size to prevent DoS
        if (jsonStr.size() > 100000000) { // 100MB limit
            throw std::runtime_error("JSON input exceeds maximum allowed size");
        }
        
        json = jsonStr.c_str();
        length = jsonStr.size();
    }
    
    // Parse and return root element
    JsonNode parse() {
        try {
            skipWhitespace();
            
            // Validation: ensure we have content to parse
            if (pos >= length) {
                throw std::runtime_error("Empty JSON document");
            }
            
            // Parse the root value
            JsonNode root = parseValue();
            
            skipWhitespace();
            
            // Validation: ensure no trailing content
            if (pos < length) {
                throw std::runtime_error("Unexpected content after JSON document");
            }
            
            return root;
        } catch (const std::exception& e) {
            // Exception handling: provide meaningful error context
            throw std::runtime_error(std::string("JSON parsing error: ") + e.what());
        }
    }
};

// Parse JSON string and return root element
JsonNode parseJsonRoot(const std::string& jsonString) {
    // Input validation delegated to JsonParser constructor
    JsonParser parser(jsonString);
    return parser.parse();
}

int main() {
    // Test case 1: Simple object
    try {
        std::string json1 = R"({"name": "John", "age": 30})";
        JsonNode root1 = parseJsonRoot(json1);
        std::cout << "Test 1 - Root type: " << root1.getTypeString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Array root
    try {
        std::string json2 = R"([1, 2, 3, "test"])";
        JsonNode root2 = parseJsonRoot(json2);
        std::cout << "Test 2 - Root type: " << root2.getTypeString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Nested structure
    try {
        std::string json3 = R"({"data": {"items": [1, 2, 3]}, "status": true})";
        JsonNode root3 = parseJsonRoot(json3);
        std::cout << "Test 3 - Root type: " << root3.getTypeString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: String root
    try {
        std::string json4 = R"("hello world")";
        JsonNode root4 = parseJsonRoot(json4);
        std::cout << "Test 4 - Root type: " << root4.getTypeString() << ", Value: " << root4.value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: Number root
    try {
        std::string json5 = "42.5";
        JsonNode root5 = parseJsonRoot(json5);
        std::cout << "Test 5 - Root type: " << root5.getTypeString() << ", Value: " << root5.value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 failed: " << e.what() << std::endl;
    }
    
    return 0;
}
