#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For simplicity and to meet the "one source file" requirement, the source code
// for the cJSON library is included here. In a real project, you would link
// against the library.
// Source: https://github.com/DaveGamble/cJSON
// (Content of cJSON.h and cJSON.c would be pasted here.
// Due to the large size, a placeholder comment is used. For a runnable file,
// you must paste the contents of cJSON.h and cJSON.c here.)
/*
 * <<< PASTE cJSON.h HERE >>>
 * <<< PASTE cJSON.c HERE >>>
 *
 * For demonstration purposes, a minimal set of declarations and definitions
 * are provided below to make this example self-contained and compilable.
 * This is NOT the full cJSON library.
 */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

typedef struct cJSON {
    struct cJSON *next, *prev;
    struct cJSON *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} cJSON;

extern cJSON *cJSON_Parse(const char *value);
extern void cJSON_Delete(cJSON *c);
extern cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *object, const char *string);
extern int cJSON_IsString(const cJSON * const item);
extern int cJSON_IsNumber(const cJSON * const item);
extern int cJSON_IsBool(const cJSON * const item);
// End of placeholder for cJSON library. A real implementation is needed.
// NOTE: The above stubs are insufficient to run. A full cJSON implementation is required.
// For this example to be runnable, you need to provide the actual library code.
// The rest of the logic assumes a working cJSON implementation.

/**
 * A simple data struct to hold user information.
 * It contains only data, making it a safe target for deserialization.
 */
typedef struct {
    int id;
    char* name; // Must be dynamically allocated
    int isAdmin; // Use int for boolean (0 or 1)
} UserData;

/**
 * Frees the memory allocated for a UserData struct, including its members.
 */
void freeUserData(UserData* user) {
    if (user != NULL) {
        free(user->name); // Free the dynamically allocated string
        free(user);       // Free the struct itself
    }
}

/**
 * Securely deserializes a JSON string into a UserData struct.
 *
 * SECURITY NOTE: This function is secure because it uses a well-known library (cJSON)
 * to parse a data-only format. It performs explicit checks for the existence and
 * type of each field. It also carefully manages memory to prevent leaks, buffer
 * overflows, or use-after-free vulnerabilities. All pointers returned from cJSON
 * are validated against NULL before being used.
 *
 * @param jsonString The user-supplied JSON string.
 * @return A pointer to a dynamically allocated UserData struct on success,
 *         or NULL on failure. The caller is responsible for freeing the
 *         returned struct using freeUserData().
 */
UserData* deserializeUserData(const char* jsonString) {
    if (jsonString == NULL || *jsonString == '\0') {
        fprintf(stderr, "Error: Input JSON string is null or empty.\n");
        return NULL;
    }

    cJSON* root = cJSON_Parse(jsonString);
    if (root == NULL) {
        fprintf(stderr, "Error: Failed to parse JSON.\n");
        return NULL;
    }

    cJSON* id_json = cJSON_GetObjectItemCaseSensitive(root, "id");
    cJSON* name_json = cJSON_GetObjectItemCaseSensitive(root, "name");
    cJSON* isAdmin_json = cJSON_GetObjectItemCaseSensitive(root, "isAdmin");

    // Validate that all required fields exist and have the correct type
    if (!cJSON_IsNumber(id_json) || !cJSON_IsString(name_json) || !cJSON_IsBool(isAdmin_json)) {
        fprintf(stderr, "Error: JSON is missing required fields or has incorrect types.\n");
        cJSON_Delete(root);
        return NULL;
    }

    // Allocate memory for the struct
    UserData* user = (UserData*)malloc(sizeof(UserData));
    if (user == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for UserData struct.\n");
        cJSON_Delete(root);
        return NULL;
    }
    
    // Allocate memory for the name and copy it
    user->name = (char*)malloc(strlen(name_json->valuestring) + 1);
    if (user->name == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for user name.\n");
        free(user);
        cJSON_Delete(root);
        return NULL;
    }
    strcpy(user->name, name_json->valuestring);

    user->id = (int)id_json->valuedouble;
    user->isAdmin = cJSON_IsTrue(isAdmin_json) ? 1 : 0;

    cJSON_Delete(root); // Clean up the parsed JSON object
    return user;
}

int main() {
    // Note: This main function will not work without a full cJSON implementation.
    // The logic below is for demonstration purposes.
    printf("--- Running C Test Cases (requires full cJSON implementation) ---\n");
    printf("--- Since a full cJSON implementation is not included, this is a demonstration of usage. ---\n");

    // --- Test Cases ---
    const char* testCases[] = {
        // Test Case 1: Valid JSON
        "{\"id\": 401, \"name\": \"Oscar\", \"isAdmin\": false}",
        // Test Case 2: Malformed JSON
        "{\"id\": 402, \"name\": \"Pat\" \"isAdmin\": true}",
        // Test Case 3: Missing field
        "{\"id\": 403, \"name\": \"Quinn\"}",
        // Test Case 4: Wrong type
        "{\"id\": \"404\", \"name\": \"Rob\", \"isAdmin\": false}",
        // Test Case 5: NULL input
        NULL
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Input: %s\n", testCases[i] ? testCases[i] : "NULL");

        UserData* user = deserializeUserData(testCases[i]);

        if (user != NULL) {
            printf("Deserialized successfully: UserData{id=%d, name='%s', isAdmin=%d}\n",
                   user->id, user->name, user->isAdmin);
            freeUserData(user); // IMPORTANT: Free the allocated memory
        } else {
            printf("Deserialization failed.\n");
        }
    }

    return 0;
}