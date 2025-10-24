#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>     // For usleep, rmdir
#include <dirent.h>     // For directory operations
#include <sys/stat.h>   // For mkdir
#include <sys/types.h>

// Global constants
#define DIR_PATH "c_test_files"
#define NUM_FILES 5 // Test case: 5 files
#define NUM_THREADS 3
#define MAX_PATH_LEN 256

// Shared data and synchronization primitive
char** g_file_paths = NULL;
int g_file_count = 0;
int g_next_file_index = 0;
pthread_mutex_t g_file_mutex;

// Forward declarations
void setup_test_environment();
void cleanup_test_environment();
void populate_file_list();

void* file_processor(void* arg) {
    long thread_id = (long)arg;
    
    while (1) {
        char current_file_path[MAX_PATH_LEN];
        int file_index = -1;

        // Lock to get the index of the next file to process
        pthread_mutex_lock(&g_file_mutex);
        if (g_next_file_index < g_file_count) {
            file_index = g_next_file_index;
            g_next_file_index++;
        }
        pthread_mutex_unlock(&g_file_mutex);

        if (file_index == -1) {
            break; // No more files to process
        }
        
        strncpy(current_file_path, g_file_paths[file_index], MAX_PATH_LEN - 1);
        current_file_path[MAX_PATH_LEN - 1] = '\0';
        
        const char *filename = strrchr(current_file_path, '/');
        filename = (filename == NULL) ? current_file_path : filename + 1;

        printf("Thread %ld is processing file: %s\n", thread_id, filename);

        usleep(100 * 1000); // Simulate processing time: 100 milliseconds

        FILE* file = fopen(current_file_path, "a");
        if (file != NULL) {
            fprintf(file, "\nProcessed by Thread %ld", thread_id);
            fclose(file);
            printf("Thread %ld finished processing file: %s\n", thread_id, filename);
        } else {
            fprintf(stderr, "Thread %ld failed to open file: %s\n", thread_id, current_file_path);
        }
    }
    return NULL;
}


void setup_test_environment() {
    printf("Setting up test environment...\n");
    // Clean up first if directory exists
    DIR* dir = opendir(DIR_PATH);
    if (dir) {
        closedir(dir);
        cleanup_test_environment(); // Call cleanup to remove old files/dir
    }

    mkdir(DIR_PATH, 0755); // Create directory with rwx for owner, rx for others

    for (int i = 1; i <= NUM_FILES; ++i) {
        char file_path[MAX_PATH_LEN];
        snprintf(file_path, MAX_PATH_LEN, "%s/test_file_%d.txt", DIR_PATH, i);
        FILE* file = fopen(file_path, "w");
        if (file != NULL) {
            fprintf(file, "This is test file %d.", i);
            fclose(file);
        }
    }
    printf("%d test files created in %s\n", NUM_FILES, DIR_PATH);
}

void cleanup_test_environment() {
    printf("\nCleaning up test environment...\n");
    // Free the global file list memory first
    if (g_file_paths != NULL) {
        for (int i = 0; i < g_file_count; ++i) {
            free(g_file_paths[i]);
        }
        free(g_file_paths);
        g_file_paths = NULL;
        g_file_count = 0;
    }

    DIR* dir = opendir(DIR_PATH);
    if (dir == NULL) return; 

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char file_path[MAX_PATH_LEN];
        snprintf(file_path, MAX_PATH_LEN, "%s/%s", DIR_PATH, entry->d_name);
        remove(file_path);
    }
    closedir(dir);

    rmdir(DIR_PATH);
    printf("Test directory and files deleted.\n");
}


void populate_file_list() {
    DIR* dir = opendir(DIR_PATH);
    if (dir == NULL) { perror("opendir"); return; }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) g_file_count++;
    }
    rewinddir(dir);

    g_file_paths = malloc(g_file_count * sizeof(char*));
    if (g_file_paths == NULL) { perror("malloc"); closedir(dir); return; }

    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < g_file_count) {
        if (entry->d_type == DT_REG) {
            char file_path[MAX_PATH_LEN];
            snprintf(file_path, MAX_PATH_LEN, "%s/%s", DIR_PATH, entry->d_name);
            g_file_paths[i] = strdup(file_path);
            if (g_file_paths[i] == NULL) {
                perror("strdup");
                for(int j=0; j<i; j++) free(g_file_paths[j]);
                free(g_file_paths);
                g_file_paths = NULL; g_file_count = 0;
                break;
            }
            i++;
        }
    }
    closedir(dir);
}

int main() {
    setup_test_environment();

    populate_file_list();
    if (g_file_count == 0) {
        fprintf(stderr, "No files found to process.\n");
        cleanup_test_environment();
        return 1;
    }

    if (pthread_mutex_init(&g_file_mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        cleanup_test_environment();
        return 1;
    }

    printf("\nStarting %d threads to process %d files.\n", NUM_THREADS, g_file_count);
    pthread_t threads[NUM_THREADS];
    for (long i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, file_processor, (void*)(i + 1)) != 0) {
            perror("pthread_create");
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nAll processing tasks are complete.\n");

    pthread_mutex_destroy(&g_file_mutex);
    cleanup_test_environment();

    return 0;
}