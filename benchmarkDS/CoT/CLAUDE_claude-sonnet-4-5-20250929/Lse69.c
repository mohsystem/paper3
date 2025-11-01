
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/types.h>
#define MKDIR(dir) mkdir(dir, 0755)
#endif

#define MAX_FILE_SIZE (5 * 1024 * 1024)
#define UPLOAD_DIR "uploads"
#define MAX_PATH_LEN 512
#define MAX_FILENAME_LEN 256

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = malloc(*output_length + 1);
    if (!encoded_data) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        unsigned int octet_a = i < input_length ? data[i++] : 0;
        unsigned int octet_b = i < input_length ? data[i++] : 0;
        unsigned int octet_c = i < input_length ? data[i++] : 0;
        unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    for (i = 0; i < (3 - input_length % 3) % 3; i++)
        encoded_data[*output_length - 1 - i] = '=';
    
    encoded_data[*output_length] = '\\0';
    return encoded_data;
}

void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int is_allowed_type(const char* filename) {
    const char* allowed[] = {".jpg", ".jpeg", ".png", ".gif"};
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    char ext_lower[10];
    strncpy(ext_lower, ext, sizeof(ext_lower) - 1);
    ext_lower[sizeof(ext_lower) - 1] = '\\0';
    to_lowercase(ext_lower);
    
    for (size_t i = 0; i < sizeof(allowed) / sizeof(allowed[0]); i++) {
        if (strcmp(ext_lower, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

char* upload_image(const unsigned char* image_data, size_t data_size, const char* original_filename) {
    if (!image_data || data_size == 0 || data_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Invalid file size\\n");
        return NULL;
    }
    
    if (!is_allowed_type(original_filename)) {
        fprintf(stderr, "File type not allowed\\n");
        return NULL;
    }
    
    MKDIR(UPLOAD_DIR);
    
    char safe_filename[MAX_PATH_LEN];
    snprintf(safe_filename, sizeof(safe_filename), "%s/uuid_%ld_%d_encoded.txt", 
             UPLOAD_DIR, (long)time(NULL), rand());
    
    size_t encoded_length;
    char* encoded_data = base64_encode(image_data, data_size, &encoded_length);
    if (!encoded_data) {
        fprintf(stderr, "Encoding failed\\n");
        return NULL;
    }
    
    FILE* file = fopen(safe_filename, "w");
    if (!file) {
        fprintf(stderr, "Error opening file\\n");
        free(encoded_data);
        return NULL;
    }
    
    fwrite(encoded_data, 1, encoded_length, file);
    fclose(file);
    free(encoded_data);
    
    char* result = malloc(strlen(safe_filename) + 1);
    if (result) {
        strcpy(result, safe_filename);
    }
    return result;
}

int main() {
    srand(time(NULL));
    
    // Test case 1: Valid small image
    const char* test1 = "test image data 1";
    char* result1 = upload_image((const unsigned char*)test1, strlen(test1), "test1.jpg");
    printf("Test 1: %s\\n", result1 ? result1 : "NULL");
    free(result1);
    
    // Test case 2: Valid PNG image
    const char* test2 = "test image data 2";
    char* result2 = upload_image((const unsigned char*)test2, strlen(test2), "test2.png");
    printf("Test 2: %s\\n", result2 ? result2 : "NULL");
    free(result2);
    
    // Test case 3: Invalid file type
    const char* test3 = "test image data 3";
    char* result3 = upload_image((const unsigned char*)test3, strlen(test3), "test3.exe");
    printf("Test 3: %s\\n", result3 ? result3 : "NULL");
    free(result3);
    
    // Test case 4: NULL data
    char* result4 = upload_image(NULL, 0, "test4.jpg");
    printf("Test 4: %s\\n", result4 ? result4 : "NULL");
    
    // Test case 5: Valid GIF image
    const char* test5 = "test image data 5";
    char* result5 = upload_image((const unsigned char*)test5, strlen(test5), "test5.gif");
    printf("Test 5: %s\\n", result5 ? result5 : "NULL");
    free(result5);
    
    return 0;
}
