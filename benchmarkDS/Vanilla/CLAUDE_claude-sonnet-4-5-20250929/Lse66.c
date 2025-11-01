
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

#define UPLOAD_DIR "uploads"
#define MAX_FILENAME 256

int create_upload_directory() {
    struct stat st = {0};
    if (stat(UPLOAD_DIR, &st) == -1) {
        return mkdir(UPLOAD_DIR, 0755) == 0;
    }
    return 1;
}

int handle_pdf_upload(const char* data, size_t data_size, const char* filename) {
    char filepath[MAX_FILENAME];
    FILE* file;
    
    if (!create_upload_directory()) {
        fprintf(stderr, "Failed to create upload directory\\n");
        return 0;
    }
    
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);
    
    file = fopen(filepath, "wb");
    if (!file) {
        fprintf(stderr, "Failed to open file for writing\\n");
        return 0;
    }
    
    fwrite(data, 1, data_size, file);
    fclose(file);
    
    return 1;
}

void run_tests() {
    printf("PDF Upload Web App - C\\n");
    printf("Testing file upload functionality...\\n");
    
    // Test case 1: Create upload directory
    if (create_upload_directory()) {
        printf("Test 1: Upload directory created/verified\\n");
    }
    
    // Test case 2-5: Upload test files
    const char* test_data[] = {
        "Test PDF content 1",
        "Test PDF content 2",
        "Test PDF content 3",
        "Test PDF content 4",
        "Test PDF content 5"
    };
    
    const char* filenames[] = {
        "test1.pdf",
        "test2.pdf",
        "test3.pdf",
        "test4.pdf",
        "test5.pdf"
    };
    
    for (int i = 0; i < 5; i++) {
        if (handle_pdf_upload(test_data[i], strlen(test_data[i]), filenames[i])) {
            printf("Test %d: File %s uploaded\\n", i + 2, filenames[i]);
        }
    }
    
    printf("\\nAll tests completed. Check 'uploads' directory for files.\\n");
}

int main() {
    run_tests();
    return 0;
}
