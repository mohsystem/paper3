
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <io.h>
    #include <sys/stat.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

int createExecutableScript(const char* fileName) {
    FILE* file = NULL;
    
    // Create and open the file for writing
    file = fopen(fileName, "w");
    
    if (file == NULL) {
        fprintf(stderr, "Error: Could not create file %s\\n", fileName);
        return 0;
    }
    
    // Write shell script header
    if (fprintf(file, "#!/bin/bash\\n") < 0 ||
        fprintf(file, "# This is an executable script\\n") < 0) {
        fprintf(stderr, "Error: Failed to write to file %s\\n", fileName);
        fclose(file);
        return 0;
    }
    
    // Flush and close the file
    if (fflush(file) != 0) {
        fprintf(stderr, "Error: Failed to flush file %s\\n", fileName);
        fclose(file);
        return 0;
    }
    
    fclose(file);
    
    // Make it executable
    #ifdef _WIN32
        // Windows - limited permission model
        if (_chmod(fileName, _S_IREAD | _S_IWRITE | _S_IEXEC) != 0) {
            fprintf(stderr, "Error: Could not set permissions on Windows\\n");
            return 0;
        }
    #else
        // Unix/Linux/Mac - use chmod with 0755 permissions
        if (chmod(fileName, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
            fprintf(stderr, "Error: Could not set executable permissions\\n");
            return 0;
        }
    #endif
    
    return 1;
}

int main() {
    printf("Test Case 1: Create script.sh\\n");
    int result1 = createExecutableScript("script.sh");
    printf("Result: %s\\n\\n", result1 ? "Success" : "Failed");
    
    printf("Test Case 2: Create test_script.sh\\n");
    int result2 = createExecutableScript("test_script.sh");
    printf("Result: %s\\n\\n", result2 ? "Success" : "Failed");
    
    printf("Test Case 3: Create another_script.sh\\n");
    int result3 = createExecutableScript("another_script.sh");
    printf("Result: %s\\n\\n", result3 ? "Success" : "Failed");
    
    printf("Test Case 4: Create deploy.sh\\n");
    int result4 = createExecutableScript("deploy.sh");
    printf("Result: %s\\n\\n", result4 ? "Success" : "Failed");
    
    printf("Test Case 5: Create backup.sh\\n");
    int result5 = createExecutableScript("backup.sh");
    printf("Result: %s\\n\\n", result5 ? "Success" : "Failed");
    
    return 0;
}
