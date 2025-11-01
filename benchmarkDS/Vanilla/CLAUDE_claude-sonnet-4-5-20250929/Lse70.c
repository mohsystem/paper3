
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#endif

#define UPLOAD_DIR "uploads"
#define MAX_FILENAME 256
#define MAX_CONTENT 1024

void create_upload_directory() {
    #ifdef _WIN32
    mkdir(UPLOAD_DIR);
    #else
    mkdir(UPLOAD_DIR, 0777);
    #endif
}

int upload_file(const char* filename, const char* content, size_t content_length) {
    create_upload_directory();
    
    char filepath[MAX_FILENAME + 20];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);
    
    FILE* file = fopen(filepath, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file for writing: %s\\n", filepath);
        return 0;
    }
    
    size_t written = fwrite(content, 1, content_length, file);
    fclose(file);
    
    if (written == content_length) {
        printf("File uploaded successfully: %s\\n", filename);
        return 1;
    } else {
        fprintf(stderr, "Error: Could not write complete file\\n");
        return 0;
    }
}

void run_tests() {
    printf("Running 5 test cases for file upload...\\n");
    
    const char* test_filenames[5] = {
        "test_file_1.txt",
        "test_file_2.txt",
        "test_file_3.txt",
        "test_file_4.txt",
        "test_file_5.txt"
    };
    
    const char* test_contents[5] = {
        "This is test file 1 content.",
        "This is test file 2 content.",
        "This is test file 3 content.",
        "This is test file 4 content.",
        "This is test file 5 content."
    };
    
    for (int i = 0; i < 5; i++) {
        printf("\\nTest case %d: ", i + 1);
        if (upload_file(test_filenames[i], test_contents[i], strlen(test_contents[i]))) {
            printf("SUCCESS\\n");
        } else {
            printf("FAILED\\n");
        }
    }
}

int main() {
    printf("File Upload Application\\n");
    printf("Upload directory: %s\\n\\n", UPLOAD_DIR);
    
    run_tests();
    
    return 0;
}
