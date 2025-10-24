
#include <stdio.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>

bool createExecutableScript(const char* filename) {
    FILE* file;
    
    // Create the file
    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating file\\n");
        return false;
    }
    
    // Write shell script header
    fprintf(file, "#!/bin/bash\\n");
    fclose(file);
    
    // Make it executable (Unix/Linux/Mac)
    #ifdef _WIN32
        // Windows - files are executable by default
        return true;
    #else
        // Unix-based systems
        if (chmod(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
            fprintf(stderr, "Error setting permissions\\n");
            return false;
        }
    #endif
    
    return true;
}

int main() {
    // Test case 1: Create script.sh
    printf("Test 1: Creating script.sh\\n");
    bool result1 = createExecutableScript("script.sh");
    printf("Result: %s\\n", result1 ? "Success" : "Failed");
    
    // Test case 2: Create test1.sh
    printf("\\nTest 2: Creating test1.sh\\n");
    bool result2 = createExecutableScript("test1.sh");
    printf("Result: %s\\n", result2 ? "Success" : "Failed");
    
    // Test case 3: Create test2.sh
    printf("\\nTest 3: Creating test2.sh\\n");
    bool result3 = createExecutableScript("test2.sh");
    printf("Result: %s\\n", result3 ? "Success" : "Failed");
    
    // Test case 4: Overwrite existing file
    printf("\\nTest 4: Creating script.sh again (overwrites)\\n");
    bool result4 = createExecutableScript("script.sh");
    printf("Result: %s\\n", result4 ? "Success" : "Failed");
    
    // Test case 5: Create another script
    printf("\\nTest 5: Creating deploy.sh\\n");
    bool result5 = createExecutableScript("deploy.sh");
    printf("Result: %s\\n", result5 ? "Success" : "Failed");
    
    // Cleanup test files
    remove("script.sh");
    remove("test1.sh");
    remove("test2.sh");
    remove("deploy.sh");
    
    return 0;
}
