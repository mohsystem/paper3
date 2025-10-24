#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_NAME_LENGTH 256

typedef struct {
    int id;
    char *name;
} UserData;

/**
 * Frees the memory allocated for a UserData struct, including the internal 'name' string.
 */
void free_user_data(UserData *user) {
    if (user != NULL) {
        free(user->name); // free the string first
        user->name = NULL;
        free(user);       // then free the struct
    }
}

/**
 * Serializes a UserData object into a simple string format into a provided buffer.
 * Format: "id=<integer>;name=<string>"
 * @param user The UserData object to serialize.
 * @param buffer The character buffer to write the serialized string to.
 * @param buffer_size The size of the buffer.
 * @return 0 on success, -1 on failure (e.g., buffer too small).
 */
int serialize(const UserData *user, char *buffer, size_t buffer_size) {
    if (user == NULL || buffer == NULL) {
        return -1;
    }
    int written = snprintf(buffer, buffer_size, "id=%d;name=%s", user->id, user->name);
    if (written < 0 || (size_t)written >= buffer_size) {
        return -1; // Encoding error or buffer too small
    }
    return 0;
}


/**
 * Deserializes a string into a UserData object.
 * This function uses a simple, custom data-only format to avoid security risks.
 * It performs careful parsing and boundary checks.
 *
 * @param data The null-terminated string to deserialize.
 * @return A pointer to a newly allocated UserData struct on success, or NULL on failure.
 *         The caller is responsible for freeing the returned struct using free_user_data().
 */
UserData* deserialize(const char *data) {
    if (data == NULL) {
        return NULL;
    }

    const char *id_prefix = "id=";
    const char *name_prefix = ";name=";

    // Find markers in the input string
    const char *id_start = strstr(data, id_prefix);
    const char *separator = strstr(data, name_prefix);

    // Basic format validation: "id=" must be at the beginning, and ";name=" must follow it.
    if (id_start != data || separator == NULL || separator < id_start) {
        return NULL;
    }

    const char *id_val_start = data + strlen(id_prefix);
    char *id_val_end;

    // Safely parse the integer ID
    errno = 0;
    long id_long = strtol(id_val_start, &id_val_end, 10);
    if (errno != 0 || id_val_end != separator || id_val_start == id_val_end) {
        return NULL; // Conversion error, or not a valid number
    }
    if (id_long > INT_MAX || id_long < INT_MIN) {
        return NULL; // ID out of range for an int
    }

    const char *name_val_start = separator + strlen(name_prefix);
    size_t name_len = strlen(name_val_start);

    // Validate name length
    if (name_len == 0 || name_len > MAX_NAME_LENGTH) {
        return NULL;
    }

    // Allocate memory for the struct and its members
    UserData *user = (UserData*)malloc(sizeof(UserData));
    if (user == NULL) {
        return NULL; // Allocation failed
    }

    user->name = (char*)malloc(name_len + 1);
    if (user->name == NULL) {
        free(user); // Clean up partially allocated struct
        return NULL; // Allocation failed
    }

    // Copy data to the new struct
    user->id = (int)id_long;
    memcpy(user->name, name_val_start, name_len);
    user->name[name_len] = '\0'; // Ensure null termination

    return user;
}

void run_test(int test_num, const char* description, const char* input) {
    printf("Test %d (%s): ", test_num, description);
    UserData *user = deserialize(input);
    if (user != NULL) {
        printf("PASSED - id=%d, name='%s'\n", user->id, user->name);
        free_user_data(user);
    } else {
        printf("FAILED (as expected)\n");
    }
}

int main() {
    printf("--- C Deserialization Tests ---\n");

    // Test Case 1: Valid data
    run_test(1, "Valid", "id=101;name=Alice");

    // Test Case 2: Invalid format (wrong order)
    run_test(2, "Wrong Order", "name=Bob;id=102");

    // Test Case 3: Invalid ID (not a number)
    run_test(3, "Invalid ID", "id=abc;name=Charlie");

    // Test Case 4: Missing part (no name)
    run_test(4, "Missing Part", "id=104");
    
    // Test Case 5: NULL input
    run_test(5, "NULL Input", NULL);

    return 0;
}