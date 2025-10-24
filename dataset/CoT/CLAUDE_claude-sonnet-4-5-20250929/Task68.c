
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 256
#define MAX_RESULT_SIZE 1024

typedef struct {
    char key[128];
    char value[512];
} KeyValue;

typedef struct {
    KeyValue items[20];
    int count;
} UserInfo;

void sanitizeLdapInput(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < outputSize - 3; i++) {
        switch (input[i]) {
            case '\\\\':\n                if (j + 3 < outputSize) {\n                    strcpy(output + j, "\\\\5c");\n                    j += 3;\n                }\n                break;\n            case '*':\n                if (j + 3 < outputSize) {\n                    strcpy(output + j, "\\\\2a");\n                    j += 3;\n                }\n                break;\n            case '(':\n                if (j + 3 < outputSize) {\n                    strcpy(output + j, "\\\\28");\n                    j += 3;\n                }\n                break;\n            case ')':\n                if (j + 3 < outputSize) {\n                    strcpy(output + j, "\\\\29");\n                    j += 3;\n                }\n                break;\n            case '\\0':\n                if (j + 3 < outputSize) {\n                    strcpy(output + j, "\\\\00");\n                    j += 3;\n                }\n                break;\n            default:\n                output[j++] = input[i];\n        }\n    }\n    output[j] = '\\0';\n}\n\nvoid sanitizeDN(const char* input, char* output, size_t outputSize) {\n    size_t j = 0;\n    size_t len = strlen(input);\n    \n    for (size_t i = 0; input[i] != '\\0' && j < outputSize - 2; i++) {\n        char c = input[i];\n        int needsEscape = 0;\n        \n        if (c == '\\\\' || c == ',' || c == '+' || c == '"' || \n            c == '<' || c == '>' || c == ';' || c == '=') {\n            needsEscape = 1;\n        } else if (c == '#' && i == 0) {\n            needsEscape = 1;\n        } else if (c == ' ' && (i == 0 || i == len - 1)) {\n            needsEscape = 1;\n        }\n        \n        if (needsEscape && j + 1 < outputSize) {\n            output[j++] = '\\\\';\n        }\n        output[j++] = c;\n    }\n    output[j] = '\\0';\n}\n\nvoid trim(char* str) {\n    char* start = str;\n    char* end;\n    \n    while (isspace((unsigned char)*start)) start++;\n    \n    if (*start == 0) {\n        str[0] = '\\0';\n        return;\n    }\n    \n    end = start + strlen(start) - 1;\n    while (end > start && isspace((unsigned char)*end)) end--;\n    \n    *(end + 1) = '\\0';\n    memmove(str, start, strlen(start) + 1);\n}\n\nint isValidDC(const char* dc) {\n    for (size_t i = 0; dc[i] != '\\0'; i++) {\n        char c = dc[i];\n        if (!isalnum(c) && c != '-' && c != '\\\\' && c != ',') {\n            return 0;\n        }\n    }\n    return 1;\n}\n\nvoid addUserInfo(UserInfo* info, const char* key, const char* value) {\n    if (info->count < 20) {\n        strncpy(info->items[info->count].key, key, 127);\n        info->items[info->count].key[127] = '\\0';\n        strncpy(info->items[info->count].value, value, 511);\n        info->items[info->count].value[511] = '\\0';\n        info->count++;\n    }\n}\n\nUserInfo searchUser(const char* username, const char* dc) {\n    UserInfo userInfo = {0};\n    char trimmedUsername[MAX_INPUT_SIZE];\n    char trimmedDC[MAX_INPUT_SIZE];\n    char sanitizedUsername[MAX_INPUT_SIZE];\n    char sanitizedDC[MAX_INPUT_SIZE];\n    \n    // Copy and trim inputs\n    strncpy(trimmedUsername, username, MAX_INPUT_SIZE - 1);\n    trimmedUsername[MAX_INPUT_SIZE - 1] = '\\0';\n    trim(trimmedUsername);\n    \n    strncpy(trimmedDC, dc, MAX_INPUT_SIZE - 1);\n    trimmedDC[MAX_INPUT_SIZE - 1] = '\\0';\n    trim(trimmedDC);\n    \n    // Validate inputs\n    if (strlen(trimmedUsername) == 0) {\n        addUserInfo(&userInfo, "error", "Username cannot be empty");\n        return userInfo;\n    }\n    \n    if (strlen(trimmedDC) == 0) {\n        addUserInfo(&userInfo, "error", "Domain component cannot be empty");\n        return userInfo;\n    }\n    \n    // Sanitize inputs\n    sanitizeLdapInput(trimmedUsername, sanitizedUsername, MAX_INPUT_SIZE);\n    sanitizeDN(trimmedDC, sanitizedDC, MAX_INPUT_SIZE);\n    \n    // Validate DC format\n    if (!isValidDC(sanitizedDC)) {\n        addUserInfo(&userInfo, "error", "Invalid domain component format");\n        return userInfo;\n    }\n    \n    /* Note: This is a demonstration of the logic flow\n     * In production, you would use OpenLDAP C API here\n     * \n     * LDAP* ld;\n     * ldap_initialize(&ld, "ldap://localhost:389");\n     * ... perform LDAP operations ...\n     * ldap_unbind_ext_s(ld, NULL, NULL);\n     */\n    \n    // Demonstration output without actual LDAP connection\n    addUserInfo(&userInfo, "note", "This is a demonstration. Requires OpenLDAP library for actual LDAP operations.");\n    addUserInfo(&userInfo, "sanitized_username", sanitizedUsername);\n    addUserInfo(&userInfo, "sanitized_dc", sanitizedDC);\n    \n    char searchFilter[MAX_RESULT_SIZE];\n    snprintf(searchFilter, MAX_RESULT_SIZE, "(uid=%s)", sanitizedUsername);\n    addUserInfo(&userInfo, "search_filter", searchFilter);\n    \n    return userInfo;\n}\n\nvoid printResult(const UserInfo* info) {\n    printf("Result:\
");\n    for (int i = 0; i < info->count; i++) {\n        printf("  %s: %s\
", info->items[i].key, info->items[i].value);\n    }\n}\n\nint main(int argc, char* argv[]) {\n    printf("=== LDAP User Search Program ===\
\
");\n    \n    // Test cases\n    const char* testCases[][2] = {\n        {"jdoe", "example,com"},\n        {"alice", "test,org"},\n        {"bob123", "company,local"},\n        {"user*", "example,com"},  // Test injection attempt\n        {"admin", "corp,internal"}\n    };\n    int numTestCases = 5;\n    \n    if (argc >= 3) {\n        // Use command line arguments\n        printf("Searching for user: %s in domain: %s\
", argv[1], argv[2]);\n        UserInfo result = searchUser(argv[1], argv[2]);\n        printResult(&result);\n    } else {\n        // Run test cases\n        printf("No command line arguments provided. Running test cases...\
\
");\n        \n        for (int i = 0; i < numTestCases; i++) {\n            printf("Test Case %d:\
", i + 1);\n            printf("Username: %s\
", testCases[i][0]);\n            printf("DC: %s\
", testCases[i][1]);\n            \n            UserInfo result = searchUser(testCases[i][0], testCases[i][1]);\n            printResult(&result);\n            printf("-----------------------------------\
\
");
        }
    }
    
    return 0;
}
