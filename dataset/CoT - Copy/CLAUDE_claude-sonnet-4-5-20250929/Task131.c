
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

typedef struct {
    char name[100];
    int age;
    char email[100];
} UserData;

int validate_email(const char* email) {
    regex_t regex;
    int result;
    
    // Simple email validation pattern
    const char* pattern = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$";
    
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return 0;
    }
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

char* extract_string_value(const char* json, const char* key) {
    static char value[100];
    char search_key[110];
    
    snprintf(search_key, sizeof(search_key), "\\"%s\\":\\"", key);
    
    const char* start = strstr(json, search_key);
    if (start == NULL) return NULL;
    
    start += strlen(search_key);
    const char* end = strchr(start, '"');
    if (end == NULL) return NULL;
    
    size_t len = end - start;
    if (len >= sizeof(value)) len = sizeof(value) - 1;
    
    strncpy(value, start, len);
    value[len] = '\\0';
    
    return value;
}

int extract_int_value(const char* json, const char* key) {
    char search_key[110];
    
    snprintf(search_key, sizeof(search_key), "\\"%s\\":", key);
    
    const char* start = strstr(json, search_key);
    if (start == NULL) return -1;
    
    start += strlen(search_key);
    return atoi(start);
}

UserData* deserialize_user_data(const char* json_data) {
    if (json_data == NULL || strlen(json_data) == 0) {
        fprintf(stderr, "Error: Input data cannot be null or empty\\n");
        return NULL;
    }
    
    UserData* user_data = (UserData*)malloc(sizeof(UserData));
    if (user_data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Extract name
    char* name = extract_string_value(json_data, "name");
    if (name == NULL || strlen(name) == 0) {
        fprintf(stderr, "Error: Name cannot be empty\\n");
        free(user_data);
        return NULL;
    }
    strncpy(user_data->name, name, sizeof(user_data->name) - 1);
    user_data->name[sizeof(user_data->name) - 1] = '\\0';
    
    // Extract age
    int age = extract_int_value(json_data, "age");
    if (age < 0 || age > 150) {
        fprintf(stderr, "Error: Age must be between 0 and 150\\n");
        free(user_data);
        return NULL;
    }
    user_data->age = age;
    
    // Extract email
    char* email = extract_string_value(json_data, "email");
    if (email == NULL || !validate_email(email)) {
        fprintf(stderr, "Error: Invalid email format\\n");
        free(user_data);
        return NULL;
    }
    strncpy(user_data->email, email, sizeof(user_data->email) - 1);
    user_data->email[sizeof(user_data->email) - 1] = '\\0';
    
    return user_data;
}

int main() {
    printf("=== Secure Deserialization Test Cases ===\\n\\n");
    
    // Test Case 1: Valid data
    const char* json1 = "{\\"name\\":\\"John Doe\\",\\"age\\":30,\\"email\\":\\"john@example.com\\"}";
    UserData* user1 = deserialize_user_data(json1);
    if (user1 != NULL) {
        printf("Test 1 - Valid data: UserData{name='%s', age=%d, email='%s'}\\n", 
               user1->name, user1->age, user1->email);
        free(user1);
    }
    
    // Test Case 2: Valid data with different values
    const char* json2 = "{\\"name\\":\\"Jane Smith\\",\\"age\\":25,\\"email\\":\\"jane@test.com\\"}";
    UserData* user2 = deserialize_user_data(json2);
    if (user2 != NULL) {
        printf("Test 2 - Valid data: UserData{name='%s', age=%d, email='%s'}\\n", 
               user2->name, user2->age, user2->email);
        free(user2);
    }
    
    // Test Case 3: Invalid age (negative)
    printf("Test 3 - Expected failure: ");
    const char* json3 = "{\\"name\\":\\"Bob\\",\\"age\\":-5,\\"email\\":\\"bob@example.com\\"}";
    UserData* user3 = deserialize_user_data(json3);
    if (user3 != NULL) free(user3);
    
    // Test Case 4: Invalid email format
    printf("Test 4 - Expected failure: ");
    const char* json4 = "{\\"name\\":\\"Alice\\",\\"age\\":28,\\"email\\":\\"invalid-email\\"}";
    UserData* user4 = deserialize_user_data(json4);
    if (user4 != NULL) free(user4);
    
    // Test Case 5: Invalid age (too high)
    printf("Test 5 - Expected failure: ");
    const char* json5 = "{\\"name\\":\\"Charlie\\",\\"age\\":200,\\"email\\":\\"charlie@test.com\\"}";
    UserData* user5 = deserialize_user_data(json5);
    if (user5 != NULL) free(user5);
    
    return 0;
}
