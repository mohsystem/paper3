
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

/* Maximum constraints to prevent resource exhaustion */
#define MAX_STRING_LENGTH 10000
#define MAX_FIELD_COUNT 100
#define MAX_INPUT_LENGTH 100000

/* Deserialized data structure */
typedef struct {
    char type;
    char* value;
    size_t valueLen;
} DeserializedData;

/* Result structure for deserialization */
typedef struct {
    DeserializedData* items;
    size_t count;
    char errorMsg[256];
} DeserializeResult;

/* Safe string to long conversion with overflow checks */
static bool safeParseLong(const char* str, long* result) {
    char* endPtr = NULL;
    
    if (str == NULL || *str == '\\0') {
        return false;
    }
    
    /* Check string length to prevent overflow */
    size_t len = strlen(str);
    if (len > 20) {
        return false;
    }
    
    errno = 0;
    *result = strtol(str, &endPtr, 10);
    
    /* Check for conversion errors */
    if (errno == ERANGE || endPtr == str || *endPtr != '\\0') {
        return false;
    }
    
    return true;
}

/* Validate that type is whitelisted */
static bool isValidType(char type) {
    return type == 'S' || type == 'I' || type == 'B';
}

/* Validate integer string */
static bool isValidInteger(const char* str, size_t len) {
    if (str == NULL || len == 0 || len > 20) {
        return false;
    }
    
    size_t start = 0;
    if (str[0] == '-') {
        if (len == 1) {
            return false;
        }
        start = 1;
    }
    
    for (size_t i = start; i < len; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return false;
        }
    }
    
    return true;
}

/* Validate boolean string */
static bool isValidBoolean(const char* str, size_t len) {
    if (str == NULL) {
        return false;
    }
    return (len == 4 && strncmp(str, "true", 4) == 0) ||
           (len == 5 && strncmp(str, "false", 5) == 0);
}

/* Free deserialization result */
static void freeDeserializeResult(DeserializeResult* result) {
    if (result == NULL) {
        return;
    }
    
    if (result->items != NULL) {
        for (size_t i = 0; i < result->count; i++) {
            if (result->items[i].value != NULL) {
                free(result->items[i].value);
                result->items[i].value = NULL;
            }
        }
        free(result->items);
        result->items = NULL;
    }
    result->count = 0;
}

/* Secure deserialization function
 * Format: TYPE:LENGTH:VALUE|TYPE:LENGTH:VALUE|...
 * Example: S:5:Hello|I:3:123|B:4:true
 */
DeserializeResult deserialize(const char* input) {
    DeserializeResult result = {NULL, 0, ""};
    
    /* Validate input pointer */
    if (input == NULL) {
        snprintf(result.errorMsg, sizeof(result.errorMsg), "Input cannot be NULL");
        return result;
    }
    
    /* Validate input length */
    size_t inputLen = strlen(input);
    if (inputLen == 0) {
        snprintf(result.errorMsg, sizeof(result.errorMsg), "Input cannot be empty");
        return result;
    }
    
    if (inputLen > MAX_INPUT_LENGTH) {
        snprintf(result.errorMsg, sizeof(result.errorMsg), "Input exceeds maximum length");
        return result;
    }
    
    /* Allocate temporary buffer for input copy (for parsing) */
    char* inputCopy = (char*)malloc(inputLen + 1);
    if (inputCopy == NULL) {
        snprintf(result.errorMsg, sizeof(result.errorMsg), "Memory allocation failed");
        return result;
    }
    memcpy(inputCopy, input, inputLen);
    inputCopy[inputLen] = '\\0';
    
    /* Allocate initial items array */
    result.items = (DeserializedData*)calloc(MAX_FIELD_COUNT, sizeof(DeserializedData));
    if (result.items == NULL) {
        free(inputCopy);
        snprintf(result.errorMsg, sizeof(result.errorMsg), "Memory allocation failed");
        return result;
    }
    
    /* Parse fields separated by '|' */
    char* savePtr = NULL;
    char* field = strtok_r(inputCopy, "|", &savePtr);
    
    while (field != NULL) {
        /* Prevent too many fields */
        if (result.count >= MAX_FIELD_COUNT) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Too many fields");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        /* Parse field: TYPE:LENGTH:VALUE */
        char* typeEnd = strchr(field, ':');
        if (typeEnd == NULL) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Invalid format - missing first colon");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        *typeEnd = '\\0';
        char* lengthStart = typeEnd + 1;
        char* lengthEnd = strchr(lengthStart, ':');
        
        if (lengthEnd == NULL) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Invalid format - missing second colon");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        *lengthEnd = '\\0';
        char* valueStart = lengthEnd + 1;
        
        /* Validate type */
        if (strlen(field) != 1 || !isValidType(field[0])) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Invalid type - must be S, I, or B");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        /* Parse and validate length */
        long declaredLength = 0;
        if (!safeParseLong(lengthStart, &declaredLength) || declaredLength < 0) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Invalid length value");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        if ((size_t)declaredLength > MAX_STRING_LENGTH) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Value exceeds maximum length");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        /* Validate actual length matches declared length */
        size_t actualLength = strlen(valueStart);
        if (actualLength != (size_t)declaredLength) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Length mismatch");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        /* Type-specific validation */
        char type = field[0];
        if (type == 'I' && !isValidInteger(valueStart, actualLength)) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Invalid integer value");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        } else if (type == 'B' && !isValidBoolean(valueStart, actualLength)) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Invalid boolean value");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        /* Allocate and copy value */
        result.items[result.count].value = (char*)malloc(actualLength + 1);
        if (result.items[result.count].value == NULL) {
            snprintf(result.errorMsg, sizeof(result.errorMsg), "Memory allocation failed");
            freeDeserializeResult(&result);
            free(inputCopy);
            return result;
        }
        
        memcpy(result.items[result.count].value, valueStart, actualLength);
        result.items[result.count].value[actualLength] = '\\0';
        result.items[result.count].type = type;
        result.items[result.count].valueLen = actualLength;
        result.count++;
        
        field = strtok_r(NULL, "|", &savePtr);
    }
    
    free(inputCopy);
    return result;
}

int main(void) {
    /* Test case 1: Valid string field */
    printf("Test 1 - String:\\n");
    DeserializeResult result1 = deserialize("S:5:Hello");
    if (result1.count > 0) {
        printf("  Value: %s\\n", result1.items[0].value);
    } else {
        printf("  Error: %s\\n", result1.errorMsg);
    }
    freeDeserializeResult(&result1);
    
    /* Test case 2: Valid integer field */
    printf("\\nTest 2 - Integer:\\n");
    DeserializeResult result2 = deserialize("I:3:123");
    if (result2.count > 0) {
        printf("  Value: %s\\n", result2.items[0].value);
    } else {
        printf("  Error: %s\\n", result2.errorMsg);
    }
    freeDeserializeResult(&result2);
    
    /* Test case 3: Multiple fields */
    printf("\\nTest 3 - Multiple fields:\\n");
    DeserializeResult result3 = deserialize("S:5:World|I:3:456|B:4:true");
    if (result3.count == 3) {
        printf("  Values: %s, %s, %s\\n", result3.items[0].value, 
               result3.items[1].value, result3.items[2].value);
    } else {
        printf("  Error: %s\\n", result3.errorMsg);
    }
    freeDeserializeResult(&result3);
    
    /* Test case 4: Invalid length mismatch (should fail) */
    printf("\\nTest 4 - Length mismatch:\\n");
    DeserializeResult result4 = deserialize("S:3:Hello");
    if (result4.count == 0) {
        printf("  Correctly rejected: %s\\n", result4.errorMsg);
    } else {
        printf("  Should have failed!\\n");
    }
    freeDeserializeResult(&result4);
    
    /* Test case 5: Invalid type (should fail) */
    printf("\\nTest 5 - Invalid type:\\n");
    DeserializeResult result5 = deserialize("X:5:Hello");
    if (result5.count == 0) {
        printf("  Correctly rejected: %s\\n", result5.errorMsg);
    } else {
        printf("  Should have failed!\\n");
    }
    freeDeserializeResult(&result5);
    
    return 0;
}
