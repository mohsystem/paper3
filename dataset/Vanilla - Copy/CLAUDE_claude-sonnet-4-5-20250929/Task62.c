
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct JSONElement {
    char type[20];
    char* stringValue;
    void* complexValue;
} JSONElement;

typedef struct ParseResult {
    JSONElement* element;
    int endIndex;
} ParseResult;

JSONElement* createJSONElement(const char* type, const char* value) {
    JSONElement* elem = (JSONElement*)malloc(sizeof(JSONElement));
    strcpy(elem->type, type);
    if (value) {
        elem->stringValue = (char*)malloc(strlen(value) + 1);
        strcpy(elem->stringValue, value);
    } else {
        elem->stringValue = NULL;
    }
    elem->complexValue = NULL;
    return elem;
}

ParseResult parseValue(const char* json, int start);

ParseResult parseString(const char* json, int start) {
    char buffer[1000];
    int bufIndex = 0;
    int i = start + 1;
    
    while (i < strlen(json) && json[i] != '"') {
        if (json[i] == '\\\\' && i + 1 < strlen(json)) i++;\n        buffer[bufIndex++] = json[i];\n        i++;\n    }\n    buffer[bufIndex] = '\\0';\n    \n    ParseResult result;\n    result.element = createJSONElement("string", buffer);\n    result.endIndex = i + 1;\n    return result;\n}\n\nParseResult parseNumber(const char* json, int start) {\n    char buffer[100];\n    int bufIndex = 0;\n    int i = start;\n    \n    while (i < strlen(json) && (isdigit(json[i]) || json[i] == '-' || json[i] == '.' || json[i] == 'e' || json[i] == 'E' || json[i] == '+')) {\n        buffer[bufIndex++] = json[i];\n        i++;\n    }\n    buffer[bufIndex] = '\\0';\n    \n    ParseResult result;\n    result.element = createJSONElement("number", buffer);\n    result.endIndex = i;\n    return result;\n}\n\nParseResult parseBoolean(const char* json, int start) {\n    ParseResult result;\n    if (strncmp(&json[start], "true", 4) == 0) {\n        result.element = createJSONElement("boolean", "true");\n        result.endIndex = start + 4;\n    } else if (strncmp(&json[start], "false", 5) == 0) {\n        result.element = createJSONElement("boolean", "false");\n        result.endIndex = start + 5;\n    } else {\n        result.element = createJSONElement("null", NULL);\n        result.endIndex = start;\n    }\n    return result;\n}\n\nParseResult parseNull(const char* json, int start) {\n    ParseResult result;\n    if (strncmp(&json[start], "null", 4) == 0) {\n        result.element = createJSONElement("null", NULL);\n        result.endIndex = start + 4;\n    } else {\n        result.element = createJSONElement("null", NULL);\n        result.endIndex = start;\n    }\n    return result;\n}\n\nParseResult parseObject(const char* json, int start) {\n    ParseResult result;\n    result.element = createJSONElement("object", "{...}");\n    int i = start + 1;\n    \n    while (i < strlen(json)) {\n        while (i < strlen(json) && isspace(json[i])) i++;\n        if (i < strlen(json) && json[i] == '}') {\n            result.endIndex = i + 1;\n            return result;\n        }\n        \n        ParseResult keyResult = parseString(json, i);\n        i = keyResult.endIndex;\n        \n        while (i < strlen(json) && isspace(json[i])) i++;\n        if (i < strlen(json) && json[i] == ':') i++;\n        \n        ParseResult valueResult = parseValue(json, i);\n        i = valueResult.endIndex;\n        \n        while (i < strlen(json) && isspace(json[i])) i++;\n        if (i < strlen(json) && json[i] == ',') i++;\n    }\n    \n    result.endIndex = i;\n    return result;\n}\n\nParseResult parseArray(const char* json, int start) {\n    ParseResult result;\n    result.element = createJSONElement("array", "[...]");\n    int i = start + 1;\n    \n    while (i < strlen(json)) {\n        while (i < strlen(json) && isspace(json[i])) i++;\n        if (i < strlen(json) && json[i] == ']') {\n            result.endIndex = i + 1;\n            return result;\n        }\n        \n        ParseResult valueResult = parseValue(json, i);\n        i = valueResult.endIndex;\n        \n        while (i < strlen(json) && isspace(json[i])) i++;\n        if (i < strlen(json) && json[i] == ',') i++;\n    }\n    \n    result.endIndex = i;\n    return result;\n}\n\nParseResult parseValue(const char* json, int start) {\n    while (start < strlen(json) && isspace(json[start])) start++;\n    \n    ParseResult result;\n    if (start >= strlen(json)) {\n        result.element = createJSONElement("null", NULL);\n        result.endIndex = start;\n        return result;\n    }\n    \n    char ch = json[start];\n    \n    if (ch == '{') {\n        return parseObject(json, start);\n    } else if (ch == '[') {\n        return parseArray(json, start);\n    } else if (ch == '"') {\n        return parseString(json, start);\n    } else if (ch == 't' || ch == 'f') {\n        return parseBoolean(json, start);\n    } else if (ch == 'n') {\n        return parseNull(json, start);\n    } else if (ch == '-' || isdigit(ch)) {\n        return parseNumber(json, start);\n    }\n    \n    result.element = createJSONElement("null", NULL);\n    result.endIndex = start;\n    return result;\n}\n\nJSONElement* parseAndGetRoot(const char* jsonString) {\n    if (!jsonString || strlen(jsonString) == 0) {\n        return createJSONElement("null", NULL);\n    }\n    \n    ParseResult result = parseValue(jsonString, 0);\n    return result.element;\n}\n\nvoid printJSONElement(JSONElement* elem) {\n    printf("JSONElement{type='%s', value=", elem->type);\n    if (elem->stringValue) {\n        printf("%s", elem->stringValue);\n    } else {\n        printf("null");\n    }\n    printf("}\
");\n}\n\nint main() {\n    // Test case 1: Simple object\n    const char* test1 = "{\\"name\\": \\"John\\", \\"age\\": 30}";
    printf("Test 1: ");
    printJSONElement(parseAndGetRoot(test1));
    
    // Test case 2: Array
    const char* test2 = "[1, 2, 3, 4, 5]";
    printf("Test 2: ");
    printJSONElement(parseAndGetRoot(test2));
    
    // Test case 3: Nested object
    const char* test3 = "{\\"person\\": {\\"name\\": \\"Alice\\", \\"age\\": 25}, \\"active\\": true}";
    printf("Test 3: ");
    printJSONElement(parseAndGetRoot(test3));
    
    // Test case 4: String value
    const char* test4 = "\\"Hello World\\"";
    printf("Test 4: ");
    printJSONElement(parseAndGetRoot(test4));
    
    // Test case 5: Complex nested structure
    const char* test5 = "{\\"users\\": [{\\"id\\": 1, \\"name\\": \\"Bob\\"}, {\\"id\\": 2, \\"name\\": \\"Carol\\"}], \\"count\\": 2}";
    printf("Test 5: ");
    printJSONElement(parseAndGetRoot(test5));
    
    return 0;
}
