
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_FILES 100
#define MAX_FILENAME 256
#define MAX_PATH 512
#define MAX_THREADS 20

typedef struct {
    char filename[MAX_FILENAME];
    pthread_mutex_t mutex;
} FileLock;

typedef struct {
    char directory_path[MAX_PATH];
    FileLock file_locks[MAX_FILES];
    int file_count;
    pthread_mutex_t locks_mutex;
} Task0;

typedef struct {
    Task0* system;
    char filename[MAX_FILENAME];
    void (*processor)(const char*);
} TaskArgs;

pthread_mutex_t* get_file_lock(Task0* system, const char* filename) {
    pthread_mutex_lock(&system->locks_mutex);
    
    for (int i = 0; i < system->file_count; i++) {
        if (strcmp(system->file_locks[i].filename, filename) == 0) {
            pthread_mutex_unlock(&system->locks_mutex);
            return &system->file_locks[i].mutex;
        }
    }
    
    if (system->file_count < MAX_FILES) {
        strcpy(system->file_locks[system->file_count].filename, filename);
        pthread_mutex_init(&system->file_locks[system->file_count].mutex, NULL);
        pthread_mutex_t* lock = &system->file_locks[system->file_count].mutex;
        system->file_count++;
        pthread_mutex_unlock(&system->locks_mutex);
        return lock;
    }
    
    pthread_mutex_unlock(&system->locks_mutex);
    return NULL;
}

void* process_file_thread(void* args) {
    TaskArgs* task_args = (TaskArgs*)args;
    Task0* system = task_args->system;
    char* filename = task_args->filename;
    void (*processor)(const char*) = task_args->processor;
    
    pthread_mutex_t* file_lock = get_file_lock(system, filename);
    
    if (file_lock && pthread_mutex_trylock(file_lock) == 0) {
        char filepath[MAX_PATH];
        snprintf(filepath, MAX_PATH, "%s/%s", system->directory_path, filename);
        
        FILE* file = fopen(filepath, "r");
        if (file) {
            fclose(file);
            processor(filepath);
            printf("Thread processed: %s\\n", filename);
        } else {
            printf("File not found: %s\\n", filename);
        }
        
        pthread_mutex_unlock(file_lock);
    } else {
        printf("Could not acquire lock for: %s\\n", filename);
    }
    
    free(task_args);
    return NULL;
}

void init_task0(Task0* system, const char* directory_path) {
    strncpy(system->directory_path, directory_path, MAX_PATH - 1);
    system->file_count = 0;
    pthread_mutex_init(&system->locks_mutex, NULL);
}

void process_file(Task0* system, const char* filename, void (*processor)(const char*)) {
    TaskArgs* args = (TaskArgs*)malloc(sizeof(TaskArgs));
    args->system = system;
    strncpy(args->filename, filename, MAX_FILENAME - 1);
    args->processor = processor;
    
    pthread_t thread;
    pthread_create(&thread, NULL, process_file_thread, args);
    pthread_detach(thread);
}

void process_all_files(Task0* system, void (*processor)(const char*)) {
    DIR* dir = opendir(system->directory_path);
    if (!dir) {
        printf("Invalid directory: %s\\n", system->directory_path);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            process_file(system, entry->d_name, processor);
        }
    }
    
    closedir(dir);
}

void cleanup_task0(Task0* system) {
    for (int i = 0; i < system->file_count; i++) {
        pthread_mutex_destroy(&system->file_locks[i].mutex);
    }
    pthread_mutex_destroy(&system->locks_mutex);
}

void test_processor(const char* filepath) {
    usleep(100000);
    FILE* file = fopen(filepath, "r");
    if (file) {
        char buffer[256];
        fgets(buffer, 256, file);
        fclose(file);
    }
}

int main() {
    char testDir[] = "test_files_c";
    mkdir(testDir, 0777);
    
    for (int i = 1; i <= 5; i++) {
        char filepath[MAX_PATH];
        snprintf(filepath, MAX_PATH, "%s/file%d.txt", testDir, i);
        FILE* file = fopen(filepath, "w");
        fprintf(file, "Test content for file %d\\n", i);
        fclose(file);
    }
    
    printf("Test Case 1: Process multiple files concurrently\\n");
    Task0 system1;
    init_task0(&system1, testDir);
    process_all_files(&system1, test_processor);
    sleep(2);
    cleanup_task0(&system1);
    
    printf("\\nTest Case 2: Multiple threads accessing same file\\n");
    Task0 system2;
    init_task0(&system2, testDir);
    for (int i = 0; i < 3; i++) {
        process_file(&system2, "file1.txt", test_processor);
    }
    sleep(2);
    cleanup_task0(&system2);
    
    printf("\\nTest Case 3: Process with file operations\\n");
    Task0 system3;
    init_task0(&system3, testDir);
    process_all_files(&system3, test_processor);
    sleep(2);
    cleanup_task0(&system3);
    
    printf("\\nTest Case 4: Process non-existent file\\n");
    Task0 system4;
    init_task0(&system4, testDir);
    process_file(&system4, "nonexistent.txt", test_processor);
    sleep(1);
    cleanup_task0(&system4);
    
    printf("\\nTest Case 5: Multiple file processing\\n");
    Task0 system5;
    init_task0(&system5, testDir);
    process_all_files(&system5, test_processor);
    sleep(2);
    cleanup_task0(&system5);
    
    DIR* dir = opendir(testDir);
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filepath[MAX_PATH];
            snprintf(filepath, MAX_PATH, "%s/%s", testDir, entry->d_name);
            remove(filepath);
        }
    }
    closedir(dir);
    rmdir(testDir);
    
    return 0;
}
