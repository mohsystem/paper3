
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#define MAX_PATH 4096
#define MAX_FILES 100
#define MAX_THREADS 10

typedef struct {
    char filePath[MAX_PATH];
    pthread_mutex_t lock;
} FileLock;

typedef struct {
    char baseDirectory[MAX_PATH];
    FileLock fileLocks[MAX_FILES];
    int fileLockCount;
    pthread_mutex_t fileLocksMapMutex;
} Task0;

char* validateAndNormalizePath(const char* path, char* result) {
    char absPath[MAX_PATH];
    if (realpath(path, absPath) == NULL) {
        mkdir(path, 0755);
        if (realpath(path, absPath) == NULL) {
            return NULL;
        }
    }
    strcpy(result, absPath);
    return result;
}

int isPathSafe(Task0* processor, const char* filePath) {
    char absPath[MAX_PATH];
    if (realpath(filePath, absPath) == NULL) {
        return 0;
    }
    return strncmp(absPath, processor->baseDirectory, strlen(processor->baseDirectory)) == 0;
}

pthread_mutex_t* getFileLock(Task0* processor, const char* fullPath) {
    pthread_mutex_lock(&processor->fileLocksMapMutex);
    
    for (int i = 0; i < processor->fileLockCount; i++) {
        if (strcmp(processor->fileLocks[i].filePath, fullPath) == 0) {
            pthread_mutex_unlock(&processor->fileLocksMapMutex);
            return &processor->fileLocks[i].lock;
        }
    }
    
    if (processor->fileLockCount < MAX_FILES) {
        strcpy(processor->fileLocks[processor->fileLockCount].filePath, fullPath);
        pthread_mutex_init(&processor->fileLocks[processor->fileLockCount].lock, NULL);
        pthread_mutex_t* lock = &processor->fileLocks[processor->fileLockCount].lock;
        processor->fileLockCount++;
        pthread_mutex_unlock(&processor->fileLocksMapMutex);
        return lock;
    }
    
    pthread_mutex_unlock(&processor->fileLocksMapMutex);
    return NULL;
}

void processFileContent(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\\n", filePath);
        return;
    }
    
    printf("Thread %lu processing: %s\\n", (unsigned long)pthread_self(), filePath);
    
    char line[1024];
    int lineCount = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        lineCount++;
    }
    
    printf("Thread %lu completed: %s (%d lines)\\n", 
           (unsigned long)pthread_self(), filePath, lineCount);
    
    fclose(file);
}

typedef struct {
    Task0* processor;
    char fileName[MAX_PATH];
} ThreadArgs;

void* processFileThread(void* args) {
    ThreadArgs* threadArgs = (ThreadArgs*)args;
    Task0* processor = threadArgs->processor;
    
    char fullPath[MAX_PATH];
    snprintf(fullPath, MAX_PATH, "%s/%s", processor->baseDirectory, threadArgs->fileName);
    
    if (!isPathSafe(processor, fullPath)) {
        fprintf(stderr, "Security violation: Path traversal attempt detected for %s\\n", 
                threadArgs->fileName);
        free(threadArgs);
        return NULL;
    }
    
    pthread_mutex_t* lock = getFileLock(processor, fullPath);
    if (lock == NULL) {
        fprintf(stderr, "Error: Too many file locks\\n");
        free(threadArgs);
        return NULL;
    }
    
    if (pthread_mutex_trylock(lock) == 0) {
        processFileContent(fullPath);
        pthread_mutex_unlock(lock);
    } else {
        fprintf(stderr, "Could not acquire lock for %s\\n", threadArgs->fileName);
    }
    
    free(threadArgs);
    return NULL;
}

void processFile(Task0* processor, const char* fileName) {
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
    args->processor = processor;
    strncpy(args->fileName, fileName, MAX_PATH - 1);
    args->fileName[MAX_PATH - 1] = '\\0';
    
    pthread_t thread;
    pthread_create(&thread, NULL, processFileThread, args);
    pthread_detach(thread);
}

Task0* createTask0(const char* baseDirectory) {
    Task0* processor = (Task0*)malloc(sizeof(Task0));
    if (!validateAndNormalizePath(baseDirectory, processor->baseDirectory)) {
        free(processor);
        return NULL;
    }
    
    processor->fileLockCount = 0;
    pthread_mutex_init(&processor->fileLocksMapMutex, NULL);
    
    return processor;
}

void destroyTask0(Task0* processor) {
    pthread_mutex_destroy(&processor->fileLocksMapMutex);
    for (int i = 0; i < processor->fileLockCount; i++) {
        pthread_mutex_destroy(&processor->fileLocks[i].lock);
    }
    free(processor);
}

int main() {
    char testDir[MAX_PATH];
    snprintf(testDir, MAX_PATH, "/tmp/test_concurrent_files");
    mkdir(testDir, 0755);
    
    for (int i = 1; i <= 5; i++) {
        char fileName[MAX_PATH];
        snprintf(fileName, MAX_PATH, "%s/test_file_%d.txt", testDir, i);
        FILE* file = fopen(fileName, "w");
        if (file) {
            for (int j = 0; j < 100; j++) {
                fprintf(file, "Test data line %d in file %d\\n", j, i);
            }
            fclose(file);
        }
    }
    
    Task0* processor = createTask0(testDir);
    if (!processor) {
        fprintf(stderr, "Failed to create processor\\n");
        return 1;
    }
    
    printf("Test Case 1: Processing multiple files concurrently\\n");
    processFile(processor, "test_file_1.txt");
    processFile(processor, "test_file_2.txt");
    processFile(processor, "test_file_3.txt");
    
    printf("\\nTest Case 2: Multiple threads attempting same file\\n");
    processFile(processor, "test_file_1.txt");
    processFile(processor, "test_file_1.txt");
    
    printf("\\nTest Case 3: Processing remaining files\\n");
    processFile(processor, "test_file_4.txt");
    processFile(processor, "test_file_5.txt");
    
    printf("\\nTest Case 4: Path traversal attack attempt\\n");
    processFile(processor, "../../../etc/passwd");
    
    printf("\\nTest Case 5: Non-existent file\\n");
    processFile(processor, "non_existent_file.txt");
    
    sleep(3);
    
    destroyTask0(processor);
    
    return 0;
}
