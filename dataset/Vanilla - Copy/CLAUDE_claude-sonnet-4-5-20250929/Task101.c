
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

int createExecutableScript(const char* filename) {
    // Create the file
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating file\\n");
        return 0;
    }
    
    fprintf(file, "#!/bin/bash\\n");
    fprintf(file, "echo 'Hello from script.sh'\\n");
    fclose(file);
    
    // Make it executable (chmod +x)
    if (chmod(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        fprintf(stderr, "Error setting permissions\\n");
        return 0;
    }
    
    return 1;
}

int main() {
    // Test case 1: Create script.sh
    printf("Test 1: %d\\n", createExecutableScript("script.sh"));
    
    // Test case 2: Create test1.sh
    printf("Test 2: %d\\n", createExecutableScript("test1.sh"));
    
    // Test case 3: Create test2.sh
    printf("Test 3: %d\\n", createExecutableScript("test2.sh"));
    
    // Test case 4: Create test3.sh
    printf("Test 4: %d\\n", createExecutableScript("test3.sh"));
    
    // Test case 5: Create test4.sh
    printf("Test 5: %d\\n", createExecutableScript("test4.sh"));
    
    return 0;
}
