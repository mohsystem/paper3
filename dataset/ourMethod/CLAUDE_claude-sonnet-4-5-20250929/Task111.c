
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

/* Security: Define maximum limits to prevent resource exhaustion */
#define MAX_REQUEST_SIZE 65536
#define MAX_RESPONSE_SIZE 65536
#define MAX_METHOD_NAME_LEN 256
#define MAX_PARAM_COUNT 32
#define MAX_PARAM_LEN 4096
#define SERVER_PORT 8080

/* Security: Structure to hold parsed XML-RPC request with bounds */
typedef struct {
    char methodName[MAX_METHOD_NAME_LEN];
    char params[MAX_PARAM_COUNT][MAX_PARAM_LEN];
    size_t paramCount;
} XMLRPCRequest;

/* Security: Safe string copy with null termination guarantee */
static void safe_strncpy(char* dest, const char* src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return;
    }
    
    /* Security: Copy with bounds check */
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\\0'; /* Security: Always null-terminate */
}

/* Security: Extract content between XML tags with strict validation */
static int extract_tag_content(const char* xml, const char* tag, size_t start_pos, 
                               char* output, size_t output_size, size_t* end_pos) {
    if (xml == NULL || tag == NULL || output == NULL || output_size == 0) {
        return 0;
    }
    
    /* Security: Initialize output buffer */
    memset(output, 0, output_size);
    
    /* Security: Build tag strings with bounds checking */
    char open_tag[128];
    char close_tag[128];
    
    /* Security: Validate tag length */
    if (strlen(tag) > 100) {
        return 0;
    }
    
    snprintf(open_tag, sizeof(open_tag), "<%s>", tag);
    snprintf(close_tag, sizeof(close_tag), "</%s>", tag);
    
    /* Security: Find opening tag with bounds check */
    const char* open_pos = strstr(xml + start_pos, open_tag);
    if (open_pos == NULL) {
        return 0;
    }
    
    const char* content_start = open_pos + strlen(open_tag);
    
    /* Security: Find closing tag with bounds check */
    const char* close_pos = strstr(content_start, close_tag);
    if (close_pos == NULL) {
        return 0;
    }
    
    /* Security: Calculate content length and validate */
    size_t content_len = close_pos - content_start;
    if (content_len == 0 || content_len >= output_size) {
        return 0;
    }
    
    /* Security: Safe copy with bounds check */
    memcpy(output, content_start, content_len);
    output[content_len] = '\\0';
    
    if (end_pos != NULL) {
        *end_pos = close_pos - xml + strlen(close_tag);
    }
    
    return 1;
}

/* Security: Validate method name - only alphanumeric and underscore */
static int validate_method_name(const char* name) {
    if (name == NULL || name[0] == '\\0') {
        return 0;
    }
    
    size_t len = strlen(name);
    if (len > MAX_METHOD_NAME_LEN - 1) {
        return 0;
    }
    
    /* Security: Check each character */
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_' && name[i] != '.') {
            return 0;
        }
    }
    
    return 1;
}

/* Security: Parse XML-RPC request with XXE prevention */
static int parse_xmlrpc_request(const char* xml, XMLRPCRequest* request) {
    if (xml == NULL || request == NULL) {
        return 0;
    }
    
    /* Security: Initialize request structure */
    memset(request, 0, sizeof(XMLRPCRequest));
    
    /* Security: Validate input size */
    size_t xml_len = strlen(xml);
    if (xml_len == 0 || xml_len > MAX_REQUEST_SIZE) {
        return 0;
    }
    
    /* Security: Check for XXE attack indicators */
    if (strstr(xml, "<!DOCTYPE") != NULL || strstr(xml, "<!ENTITY") != NULL) {
        return 0; /* Reject requests with DOCTYPE/ENTITY declarations */
    }
    
    /* Security: Extract method name with validation */
    char method_buffer[MAX_METHOD_NAME_LEN];
    size_t pos = 0;
    
    if (!extract_tag_content(xml, "methodName", 0, method_buffer, 
                            sizeof(method_buffer), &pos)) {
        return 0;
    }
    
    /* Security: Validate method name */
    if (!validate_method_name(method_buffer)) {
        return 0;
    }
    
    safe_strncpy(request->methodName, method_buffer, sizeof(request->methodName));
    
    /* Security: Parse parameters with count limit */
    const char* params_start = strstr(xml, "<params>");
    if (params_start != NULL) {
        size_t param_pos = params_start - xml;
        
        while (request->paramCount < MAX_PARAM_COUNT) {
            char value_buffer[MAX_PARAM_LEN];
            size_t next_pos;
            
            if (!extract_tag_content(xml, "value", param_pos, value_buffer, 
                                    sizeof(value_buffer), &next_pos)) {
                break;
            }
            
            safe_strncpy(request->params[request->paramCount], value_buffer, 
                        sizeof(request->params[request->paramCount]));
            request->paramCount++;
            param_pos = next_pos;
        }
    }
    
    return 1;
}

/* Security: XML escape function to prevent injection */
static void xml_escape(const char* input, char* output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) {
        return;
    }
    
    /* Security: Initialize output */
    memset(output, 0, output_size);
    
    size_t out_pos = 0;
    size_t in_len = strlen(input);
    
    for (size_t i = 0; i < in_len && out_pos < output_size - 6; i++) {
        /* Security: Escape XML special characters */
        switch (input[i]) {
            case '&':
                if (out_pos + 5 < output_size) {
                    memcpy(output + out_pos, "&amp;", 5);
                    out_pos += 5;
                }
                break;
            case '<':
                if (out_pos + 4 < output_size) {
                    memcpy(output + out_pos, "&lt;", 4);
                    out_pos += 4;
                }
                break;
            case '>':
                if (out_pos + 4 < output_size) {
                    memcpy(output + out_pos, "&gt;", 4);
                    out_pos += 4;
                }
                break;
            case '"':
                if (out_pos + 6 < output_size) {
                    memcpy(output + out_pos, "&quot;", 6);
                    out_pos += 6;
                }
                break;
            case '\\'':
                if (out_pos + 6 < output_size) {
                    memcpy(output + out_pos, "&apos;", 6);
                    out_pos += 6;
                }
                break;
            default:
                /* Security: Only allow printable ASCII */
                if (input[i] >= 32 && input[i] <= 126) {
                    output[out_pos++] = input[i];
                }
                break;
        }
    }
    
    output[out_pos] = '\\0';
}

/* Security: Build success response with safe formatting */
static void build_success_response(const char* result, char* output, size_t output_size) {
    if (result == NULL || output == NULL || output_size == 0) {
        return;
    }
    
    /* Security: Escape result */
    char escaped[MAX_RESPONSE_SIZE / 2];
    xml_escape(result, escaped, sizeof(escaped));
    
    /* Security: Use snprintf for safe formatting */
    snprintf(output, output_size,
        "<?xml version=\\"1.0\\"?>\\r\\n"
        "<methodResponse>\\r\\n"
        "  <params>\\r\\n"
        "    <param>\\r\\n"
        "      <value><string>%s</string></value>\\r\\n"
        "    </param>\\r\\n"
        "  </params>\\r\\n"
        "</methodResponse>\\r\\n",
        escaped);
}

/* Security: Build fault response with safe formatting */
static void build_fault_response(int code, const char* message, char* output, size_t output_size) {
    if (message == NULL || output == NULL || output_size == 0) {
        return;
    }
    
    /* Security: Escape message */
    char escaped[1024];
    xml_escape(message, escaped, sizeof(escaped));
    
    /* Security: Use snprintf for safe formatting */
    snprintf(output, output_size,
        "<?xml version=\\"1.0\\"?>\\r\\n"
        "<methodResponse>\\r\\n"
        "  <fault>\\r\\n"
        "    <value>\\r\\n"
        "      <struct>\\r\\n"
        "        <member>\\r\\n"
        "          <name>faultCode</name>\\r\\n"
        "          <value><int>%d</int></value>\\r\\n"
        "        </member>\\r\\n"
        "        <member>\\r\\n"
        "          <name>faultString</name>\\r\\n"
        "          <value><string>%s</string></value>\\r\\n"
        "        </member>\\r\\n"
        "      </struct>\\r\\n"
        "    </value>\\r\\n"
        "  </fault>\\r\\n"
        "</methodResponse>\\r\\n",
        code, escaped);
}

/* Security: Safe string to integer conversion with overflow check */
static int safe_strtoi(const char* str, int* result) {
    if (str == NULL || result == NULL) {
        return 0;
    }
    
    char* endptr;
    long val = strtol(str, &endptr, 10);
    
    /* Security: Check for conversion errors */
    if (endptr == str || *endptr != '\\0') {
        return 0;
    }
    
    /* Security: Check for overflow */
    if (val > INT_MAX || val < INT_MIN) {
        return 0;
    }
    
    *result = (int)val;
    return 1;
}

/* Security: Process method with input validation and overflow checks */
static void process_method(const XMLRPCRequest* request, char* response, size_t response_size) {
    if (request == NULL || response == NULL || response_size == 0) {
        return;
    }
    
    /* Security: Whitelist allowed methods */
    if (strcmp(request->methodName, "add") == 0) {
        if (request->paramCount != 2) {
            build_fault_response(2, "add requires exactly 2 parameters", response, response_size);
            return;
        }
        
        int a, b;
        
        /* Security: Validate numeric input */
        if (!safe_strtoi(request->params[0], &a) || !safe_strtoi(request->params[1], &b)) {
            build_fault_response(4, "Invalid numeric parameters", response, response_size);
            return;
        }
        
        /* Security: Check for integer overflow */
        if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) {
            build_fault_response(3, "Integer overflow", response, response_size);
            return;
        }
        
        int result = a + b;
        char result_str[32];
        snprintf(result_str, sizeof(result_str), "%d", result);
        build_success_response(result_str, response, response_size);
        
    } else if (strcmp(request->methodName, "echo") == 0) {
        if (request->paramCount < 1) {
            build_fault_response(2, "echo requires at least 1 parameter", response, response_size);
            return;
        }
        build_success_response(request->params[0], response, response_size);
        
    } else if (strcmp(request->methodName, "getTime") == 0) {
        time_t now = time(NULL);
        char time_str[32];
        snprintf(time_str, sizeof(time_str), "%ld", (long)now);
        build_success_response(time_str, response, response_size);
        
    } else {
        build_fault_response(1, "Method not found", response, response_size);
    }
}

int main(void) {
    printf("XML-RPC Server Test Cases\\n");
    printf("=========================\\n\\n");
    
    /* Test Case 1: Valid add method */
    printf("Test 1: Valid add method\\n");
    XMLRPCRequest req1;
    const char* xml1 = "<?xml version=\\"1.0\\"?><methodCall><methodName>add</methodName>"
                       "<params><param><value>5</value></param><param><value>3</value></param></params></methodCall>";
    if (parse_xmlrpc_request(xml1, &req1)) {
        char response1[MAX_RESPONSE_SIZE];
        process_method(&req1, response1, sizeof(response1));
        printf("Result: %s\\n\\n", strstr(response1, "<string>8</string>") ? "PASS" : "FAIL");
    }
    
    /* Test Case 2: Echo method */
    printf("Test 2: Echo method\\n");
    XMLRPCRequest req2;
    const char* xml2 = "<?xml version=\\"1.0\\"?><methodCall><methodName>echo</methodName>"
                       "<params><param><value>Hello World</value></param></params></methodCall>";
    if (parse_xmlrpc_request(xml2, &req2)) {
        char response2[MAX_RESPONSE_SIZE];
        process_method(&req2, response2, sizeof(response2));
        printf("Result: %s\\n\\n", strstr(response2, "Hello World") ? "PASS" : "FAIL");
    }
    
    /* Test Case 3: Invalid method */
    printf("Test 3: Invalid method\\n");
    XMLRPCRequest req3;
    const char* xml3 = "<?xml version=\\"1.0\\"?><methodCall><methodName>invalidMethod</methodName>"
                       "<params></params></methodCall>";
    if (parse_xmlrpc_request(xml3, &req3)) {
        char response3[MAX_RESPONSE_SIZE];
        process_method(&req3, response3, sizeof(response3));
        printf("Result: %s\\n\\n", strstr(response3, "Method not found") ? "PASS" : "FAIL");
    }
    
    /* Test Case 4: XXE attack prevention */
    printf("Test 4: XXE attack prevention\\n");
    XMLRPCRequest req4;
    const char* xml4 = "<?xml version=\\"1.0\\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \\"file:///etc/passwd\\">]>"
                       "<methodCall><methodName>echo</methodName><params><param><value>&xxe;</value></param></params></methodCall>";
    int parsed4 = parse_xmlrpc_request(xml4, &req4);
    printf("Result: %s\\n\\n", !parsed4 ? "PASS (rejected)" : "FAIL");
    
    /* Test Case 5: Integer overflow protection */
    printf("Test 5: Integer overflow protection\\n");
    XMLRPCRequest req5;
    const char* xml5 = "<?xml version=\\"1.0\\"?><methodCall><methodName>add</methodName>"
                       "<params><param><value>2147483647</value></param><param><value>1</value></param></params></methodCall>";
    if (parse_xmlrpc_request(xml5, &req5)) {
        char response5[MAX_RESPONSE_SIZE];
        process_method(&req5, response5, sizeof(response5));
        printf("Result: %s\\n\\n", strstr(response5, "overflow") ? "PASS" : "FAIL");
    }
    
    printf("All test cases completed.\\n");
    printf("Note: Full server requires socket initialization and listening.\\n");
    
    return 0;
}
