#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple struct to hold user data
typedef struct {
    int id;
    char* name;
} UserData;

/**
 * Helper function to serialize UserData into a byte buffer.
 * The caller is responsible for freeing the returned buffer.
 * Format: [id (int)][name_length (size_t)][name_data (char*)]
 * @param user Pointer to the UserData struct to serialize.
 * @param out_size Pointer to a size_t to store the size of the output buffer.
 * @return A dynamically allocated buffer with the serialized data.
 */
char* serialize_data(const UserData* user, size_t* out_size) {
    size_t name_len = strlen(user->name);
    *out_size = sizeof(user->id) + sizeof(size_t) + name_len;
    
    char* buffer = (char*)malloc(*out_size);
    if (!buffer) {
        perror("Failed to allocate memory for serialization");
        return NULL;
    }

    char* ptr = buffer;

    // Copy ID
    memcpy(ptr, &user->id, sizeof(user->id));
    ptr += sizeof(user->id);

    // Copy name length
    memcpy(ptr, &name_len, sizeof(size_t));
    ptr += sizeof(size_t);

    // Copy name
    memcpy(ptr, user->name, name_len);

    return buffer;
}

/**
 * Deserializes user-supplied byte buffer back into a UserData struct.
 * The caller is responsible for freeing the returned UserData struct and its members.
 * @param data The buffer containing the serialized data.
 * @param data_size The size of the data buffer.
 * @return A pointer to a new dynamically allocated UserData struct, or NULL on error.
 */
UserData* deserialize_data(const char* data, size_t data_size) {
    if (data == NULL || data_size < sizeof(int) + sizeof(size_t)) {
        fprintf(stderr, "Error: Invalid data or data size for deserialization.\n");
        return NULL;
    }

    UserData* user = (UserData*)malloc(sizeof(UserData));
    if (!user) {
        perror("Failed to allocate memory for UserData struct");
        return NULL;
    }

    const char* ptr = data;
    
    // Read ID
    memcpy(&user->id, ptr, sizeof(user->id));
    ptr += sizeof(user->id);

    // Read name length
    size_t name_len;
    memcpy(&name_len, ptr, sizeof(size_t));
    ptr += sizeof(size_t);
    
    // Sanity check
    if (data_size < (ptr - data) + name_len) {
        fprintf(stderr, "Error: Buffer is smaller than expected based on name length.\n");
        free(user);
        return NULL;
    }
    
    // Allocate memory for name and copy it
    user->name = (char*)malloc(name_len + 1); // +1 for null terminator
    if (!user->name) {
        perror("Failed to allocate memory for user name");
        free(user);
        return NULL;
    }
    memcpy(user->name, ptr, name_len);
    user->name[name_len] = '\0'; // Null-terminate the string

    return user;
}

// Helper to free a UserData struct
void free_user_data(UserData* user) {
    if (user) {
        free(user->name);
        free(user);
    }
}

void run_test(int id, const char* name) {
    static int test_num = 1;
    printf("\n--- Test Case %d ---\n", test_num++);

    UserData original_user = {id, (char*)name};
    printf("Original Object: { id: %d, name: \"%s\" }\n", original_user.id, original_user.name);

    // 1. Serialize
    size_t serialized_size;
    char* serialized_data = serialize_data(&original_user, &serialized_size);
    if (!serialized_data) return;
    printf("Serialized Data (size): %zu bytes\n", serialized_size);

    // 2. Deserialize
    UserData* deserialized_user = deserialize_data(serialized_data, serialized_size);
    
    if (deserialized_user) {
        printf("Deserialized Object: { id: %d, name: \"%s\" }\n", deserialized_user->id, deserialized_user->name);
    } else {
        printf("Deserialization resulted in NULL.\n");
    }

    // Cleanup
    free(serialized_data);
    free_user_data(deserialized_user);
}

int main() {
    printf("--- C Deserialization Test ---\n");
    
    // 5 Test Cases
    run_test(301, "Alice");
    run_test(302, "Bob");
    run_test(303, "Charlie");
    run_test(304, "David has a much longer name");
    run_test(305, ""); // Empty name

    return 0;
}