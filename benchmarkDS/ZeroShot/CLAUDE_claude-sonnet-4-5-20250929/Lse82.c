
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: C doesn't have built-in web server or JSON parsing\n// This is a simplified version showing the core logic\n\nint hexToInt(char c) {\n    if (c >= '0' && c <= '9') return c - '0';\n    if (c >= 'a' && c <= 'f') return c - 'a' + 10;\n    if (c >= 'A' && c <= 'F') return c - 'A' + 10;\n    return 0;\n}\n\nvoid urlDecode(const char* src, char* dest) {\n    int i = 0, j = 0;\n    while (src[i]) {\n        if (src[i] == '%' && src[i+1] && src[i+2]) {\n            dest[j++] = hexToInt(src[i+1]) * 16 + hexToInt(src[i+2]);\n            i += 3;\n        } else if (src[i] == '+') {\n            dest[j++] = ' ';\n            i++;\n        } else {\n            dest[j++] = src[i++];\n        }\n    }\n    dest[j] = '\\0';\n}\n\nchar* extractJsonValue(const char* json, const char* key, char* result) {\n    char searchKey[256];\n    sprintf(searchKey, "\\"%s\\":\\"", key);\n    \n    char* pos = strstr(json, searchKey);\n    if (pos != NULL) {\n        pos += strlen(searchKey);\n        char* endPos = strchr(pos, '"');\n        if (endPos != NULL) {\n            int len = endPos - pos;\n            strncpy(result, pos, len);\n            result[len] = '\\0';\n            return result;\n        }\n    }\n    strcpy(result, "Name not found");\n    return result;\n}\n\nchar* processPayload(const char* payload, char* output) {\n    char decoded[1024];\n    char result[256];\n    \n    urlDecode(payload, decoded);\n    extractJsonValue(decoded, "name", result);\n    strcpy(output, result);\n    \n    return output;\n}\n\nint main() {\n    char output[256];\n    \n    printf("Test Cases:\
");\n    printf("1. %s\
", processPayload("%7B%22name%22%3A%22John%22%7D", output));\n    printf("2. %s\
", processPayload("%7B%22name%22%3A%22Alice%22%2C%22age%22%3A30%7D", output));\n    printf("3. %s\
", processPayload("%7B%22age%22%3A25%7D", output));\n    printf("4. %s\
", processPayload("%7B%22name%22%3A%22Bob%20Smith%22%7D", output));\n    printf("5. %s\
", processPayload("invalid", output));\n    \n    printf("\
Note: Full web server implementation requires external libraries\
");
    
    return 0;
}
