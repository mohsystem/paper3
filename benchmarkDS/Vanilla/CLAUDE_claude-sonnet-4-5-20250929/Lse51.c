
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

void unzipTarArchive(const char* tarFilePath, const char* destDirectory) {
    char mkdirCmd[512];
    char extractCmd[512];
    
    // Create destination directory
    snprintf(mkdirCmd, sizeof(mkdirCmd), "mkdir -p %s", destDirectory);
    system(mkdirCmd);
    
    // Determine compression type and extract
    if (strstr(tarFilePath, ".tar.gz") != NULL || strstr(tarFilePath, ".tgz") != NULL) {
        snprintf(extractCmd, sizeof(extractCmd), "tar -xzf %s -C %s", tarFilePath, destDirectory);
    } else if (strstr(tarFilePath, ".tar.bz2") != NULL) {
        snprintf(extractCmd, sizeof(extractCmd), "tar -xjf %s -C %s", tarFilePath, destDirectory);
    } else if (strstr(tarFilePath, ".tar") != NULL) {
        snprintf(extractCmd, sizeof(extractCmd), "tar -xf %s -C %s", tarFilePath, destDirectory);
    } else {
        fprintf(stderr, "Unsupported file format\\n");
        return;
    }
    
    system(extractCmd);
}

char* openFile(const char* filePath, const char* mode) {
    FILE* file = fopen(filePath, mode);
    if (file == NULL) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc(fileSize + 1);
    if (content) {
        fread(content, 1, fileSize, file);
        content[fileSize] = '\\0';
    }
    
    fclose(file);
    return content;
}

int main() {
    const char* testCases[] = {
        "test1.tar.gz",
        "test2.tar.bz2",
        "test3.tar",
        "test4.tgz",
        "test5.tar.gz"
    };
    
    int numTests = sizeof(testCases) / sizeof(testCases[0]);
    
    for (int i = 0; i < numTests; i++) {
        printf("Extracting: %s\\n", testCases[i]);
        unzipTarArchive(testCases[i], "/tmp/unpack");
        printf("Extraction command executed for: %s\\n", testCases[i]);
    }
    
    return 0;
}
