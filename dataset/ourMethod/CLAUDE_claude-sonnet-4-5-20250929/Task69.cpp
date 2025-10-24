
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

// Validates that the path is within allowed base directory and is a regular file
// Returns true if valid, false otherwise
bool validateFilePath(const char* basePath, const char* userPath, char* resolvedPath, size_t resolvedSize) {
    // Input validation: check for null pointers
    if (!basePath || !userPath || !resolvedPath) {
        std::cerr << "Error: Null pointer provided to validateFilePath\\n";
        return false;
    }
    
    // Validate input length to prevent buffer overflow
    if (strlen(userPath) > PATH_MAX) {
        std::cerr << "Error: Path too long\\n";
        return false;
    }
    
    // Check for dangerous path characters and sequences
    if (strstr(userPath, "..") != nullptr) {
        std::cerr << "Error: Path traversal detected (..)\\n";
        return false;
    }
    
    // Resolve base path to canonical form
    char resolvedBase[PATH_MAX];
    if (realpath(basePath, resolvedBase) == nullptr) {
        std::cerr << "Error: Cannot resolve base directory\\n";
        return false;
    }
    
    // Resolve user path to canonical form
    if (realpath(userPath, resolvedPath) == nullptr) {
        std::cerr << "Error: Cannot resolve user path or file does not exist\\n";
        return false;
    }
    
    // Ensure resolved path starts with base directory
    size_t baseLen = strlen(resolvedBase);
    if (strncmp(resolvedPath, resolvedBase, baseLen) != 0) {
        std::cerr << "Error: Path escapes base directory\\n";
        return false;
    }
    
    // Ensure there's a path separator after base (unless path equals base)\n    if (strlen(resolvedPath) > baseLen && resolvedPath[baseLen] != '/') {\n        std::cerr << "Error: Invalid path structure\\n";\n        return false;\n    }\n    \n    // Check if path is a regular file (not directory or symlink)\n    struct stat statbuf;\n    if (lstat(resolvedPath, &statbuf) != 0) {\n        std::cerr << "Error: Cannot stat file\\n";\n        return false;\n    }\n    \n    // Reject symlinks to prevent TOCTOU attacks\n    if (S_ISLNK(statbuf.st_mode)) {\n        std::cerr << "Error: Symlinks are not allowed\\n";\n        return false;\n    }\n    \n    // Ensure it's a regular file
    if (!S_ISREG(statbuf.st_mode)) {
        std::cerr << "Error: Not a regular file\\n";
        return false;
    }
    
    return true;
}

// Safely deletes a file with validation
bool deleteFile(const char* basePath, const char* filePath) {
    char resolvedPath[PATH_MAX];
    
    // Validate the file path before deletion
    if (!validateFilePath(basePath, filePath, resolvedPath, sizeof(resolvedPath))) {
        return false;
    }
    
    // Perform atomic deletion using unlink
    // Using unlink instead of remove to be explicit about deleting files only
    if (unlink(resolvedPath) != 0) {
        std::cerr << "Error: Failed to delete file: " << strerror(errno) << "\\n";
        return false;
    }
    
    std::cout << "File deleted successfully: " << resolvedPath << "\\n";
    return true;
}

int main(int argc, char* argv[]) {
    // Validate command line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << (argv[0] ? argv[0] : "program") << " <file_path>\\n";
        return 1;
    }
    
    // Get base directory (current working directory for this example)
    char baseDir[PATH_MAX];
    if (getcwd(baseDir, sizeof(baseDir)) == nullptr) {
        std::cerr << "Error: Cannot get current directory\\n";
        return 1;
    }
    
    // Delete the specified file
    if (!deleteFile(baseDir, argv[1])) {
        return 1;
    }
    
    // Test cases (commented out for production use)
    // Uncomment to run tests - these would need test files created first
    /*
    std::cout << "\\n=== Running Test Cases ===\\n";
    
    // Test 1: Valid file in current directory
    std::cout << "\\nTest 1: Delete valid file\\n";
    FILE* f1 = fopen("test1.txt", "w");
    if (f1) { fprintf(f1, "test"); fclose(f1); }
    deleteFile(baseDir, "test1.txt");
    
    // Test 2: Attempt path traversal
    std::cout << "\\nTest 2: Path traversal attempt (should fail)\\n";
    deleteFile(baseDir, "../etc/passwd");
    
    // Test 3: Non-existent file
    std::cout << "\\nTest 3: Non-existent file (should fail)\\n";
    deleteFile(baseDir, "nonexistent.txt");
    
    // Test 4: Attempt to delete directory
    std::cout << "\\nTest 4: Attempt to delete directory (should fail)\\n";
    deleteFile(baseDir, ".");
    
    // Test 5: Valid file with absolute path
    std::cout << "\\nTest 5: Valid file with absolute path\\n";
    char testPath[PATH_MAX];
    snprintf(testPath, sizeof(testPath), "%s/test5.txt", baseDir);
    FILE* f5 = fopen(testPath, "w");
    if (f5) { fprintf(f5, "test"); fclose(f5); }
    deleteFile(baseDir, testPath);
    */
    
    return 0;
}
