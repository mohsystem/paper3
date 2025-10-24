
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Deserialize integer from string
int deserialize_int(const char* data) {
    return atoi(data);
}

// Deserialize float from string
float deserialize_float(const char* data) {
    return atof(data);
}

// Deserialize string (copy)
char* deserialize_string(const char* data) {
    char* result = (char*)malloc(strlen(data) + 1);
    if (result) {
        strcpy(result, data);
    }
    return result;
}

// Deserialize from file
char* deserialize_from_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    if (buffer) {
        fread(buffer, 1, size, file);
        buffer[size] = '\\0';
    }
    
    fclose(file);
    return buffer;
}

// Serialize integer to string (for testing)
char* serialize_int(int value) {
    char* buffer = (char*)malloc(20);
    if (buffer) {
        sprintf(buffer, "%d", value);
    }
    return buffer;
}

// Serialize float to string (for testing)
char* serialize_float(float value) {
    char* buffer = (char*)malloc(20);
    if (buffer) {
        sprintf(buffer, "%f", value);
    }
    return buffer;
}

int main() {
    // Test Case 1: Deserialize integer
    printf("Test Case 1: Integer\\n");
    int num = 42;
    char* serialized1 = serialize_int(num);
    int deserialized1 = deserialize_int(serialized1);
    printf("Original: %d\\n", num);
    printf("Deserialized: %d\\n\\n", deserialized1);
    free(serialized1);
    
    // Test Case 2: Deserialize string
    printf("Test Case 2: String\\n");
    const char* str = "Hello, World!";
    char* deserialized2 = deserialize_string(str);
    printf("Original: %s\\n", str);
    printf("Deserialized: %s\\n\\n", deserialized2);
    free(deserialized2);
    
    // Test Case 3: Deserialize float
    printf("Test Case 3: Float\\n");
    float flt = 3.14159f;
    char* serialized3 = serialize_float(flt);
    float deserialized3 = deserialize_float(serialized3);
    printf("Original: %f\\n", flt);
    printf("Deserialized: %f\\n\\n", deserialized3);
    free(serialized3);
    
    // Test Case 4: Deserialize negative integer
    printf("Test Case 4: Negative Integer\\n");
    int neg = -100;
    char* serialized4 = serialize_int(neg);
    int deserialized4 = deserialize_int(serialized4);
    printf("Original: %d\\n", neg);
    printf("Deserialized: %d\\n\\n", deserialized4);
    free(serialized4);
    
    // Test Case 5: Deserialize large integer
    printf("Test Case 5: Large Integer\\n");
    int large = 1234567;
    char* serialized5 = serialize_int(large);
    int deserialized5 = deserialize_int(serialized5);
    printf("Original: %d\\n", large);
    printf("Deserialized: %d\\n", deserialized5);
    free(serialized5);
    
    return 0;
}
