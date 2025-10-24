#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define NUM_THREADS 4
#define MAX_FILENAME_LEN 256

// Shared work queue structure
typedef struct {
    char **filenames;
    int file_count;
    volatile int next_file_index;
    pthread_mutex_t mutex;
    int dir_fd;
} WorkQueue;

// Arguments for each thread
typedef struct {
    int thread_id;
    WorkQueue *queue;
} ThreadArg;

// Function prototypes
void process_file(int dir_fd, const char *filename, int thread_id);
void *worker_thread(void *arg);
int populate_work_queue(WorkQueue *queue, const char *dir_path);
void cleanup_work_queue(WorkQueue *queue);
void run_test(const char *test_name, const char *dir_path);
int setup_test_directory(const char* dir_path);
void cleanup_test_directory(const char* dir_path);

/**
 * @brief Simulates processing a single file.
 *
 * This function opens a file using its descriptor relative to a directory descriptor,
 * verifies it is a regular file, and then simulates work.
 * This approach is secure against TOCTOU races and path traversal.
 *
 * @param dir_fd The file descriptor of the parent directory.
 * @param filename The name of the file to process.
 * @param thread_id The ID of the calling thread for logging.
 */
void process_file(int dir_fd, const char *filename, int thread_id) {
    if (filename == NULL) {
        return;
    }

    printf("[Thread %d] Starting to process file: %s\n", thread_id, filename);

    // O_NOFOLLOW: Do not follow symbolic links.
    // O_CLOEXEC: The file descriptor will be closed automatically on exec.
    int fd = openat(dir_fd, filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd == -1) {
        fprintf(stderr, "[Thread %d] Error: Failed to open file %s: %s\n", thread_id, filename, strerror(errno));
        return;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        fprintf(stderr, "[Thread %d] Error: Failed to fstat file %s: %s\n", thread_id, filename, strerror(errno));
        close(fd);
        return;
    }

    // Ensure we are only processing regular files.
    if (!S_ISREG(st.st_mode)) {
        printf("[Thread %d] Skipping non-regular file: %s\n", thread_id, filename);
        close(fd);
        return;
    }

    // Simulate file processing
    printf("[Thread %d] Processing file %s (size: %ld bytes)\n", thread_id, filename, (long)st.st_size);
    // In a real application, file content would be read and processed here.
    usleep(100000); // Sleep for 100ms to simulate work

    printf("[Thread %d] Finished processing file: %s\n", thread_id, filename);

    close(fd);
}

/**
 * @brief The main function for each worker thread.
 *
 * Threads loop, pulling filenames from the shared queue and processing them
 * until no files are left.
 *
 * @param arg A pointer to a ThreadArg struct.
 * @return NULL.
 */
void *worker_thread(void *arg) {
    ThreadArg *thread_arg = (ThreadArg *)arg;
    WorkQueue *queue = thread_arg->queue;
    int thread_id = thread_arg->thread_id;
    char *filename_to_process = NULL;

    while (1) {
        filename_to_process = NULL;

        // Lock the mutex to safely get the next file index
        pthread_mutex_lock(&queue->mutex);

        if (queue->next_file_index < queue->file_count) {
            filename_to_process = queue->filenames[queue->next_file_index];
            queue->next_file_index++;
        }

        pthread_mutex_unlock(&queue->mutex);

        if (filename_to_process == NULL) {
            // No more files to process
            break;
        }

        process_file(queue->dir_fd, filename_to_process, thread_id);
    }

    return NULL;
}

/**
 * @brief Scans a directory and populates the work queue with regular file names.
 *
 * @param queue The WorkQueue to populate.
 * @param dir_path The path to the directory to scan.
 * @return 0 on success, -1 on failure.
 */
int populate_work_queue(WorkQueue *queue, const char *dir_path) {
    DIR *dir_stream = opendir(dir_path);
    if (dir_stream == NULL) {
        fprintf(stderr, "Error: Could not open directory %s: %s\n", dir_path, strerror(errno));
        return -1;
    }

    // The dir_fd remains valid even after closedir()
    queue->dir_fd = dirfd(dir_stream);
    if (queue->dir_fd == -1) {
        fprintf(stderr, "Error: Could not get directory file descriptor for %s: %s\n", dir_path, strerror(errno));
        closedir(dir_stream);
        return -1;
    }

    queue->file_count = 0;
    queue->filenames = NULL;
    struct dirent *entry;

    // First pass: count regular files to allocate memory
    while ((entry = readdir(dir_stream)) != NULL) {
        if (entry->d_type == DT_REG) {
            queue->file_count++;
        } else if (entry->d_type == DT_UNKNOWN) {
            // Filesystem doesn't support d_type, need to stat
            struct stat st;
            if (fstatat(queue->dir_fd, entry->d_name, &st, AT_NO_AUTOMOUNT) == 0) {
                if (S_ISREG(st.st_mode)) {
                    queue->file_count++;
                }
            }
        }
    }

    if (queue->file_count == 0) {
        printf("No regular files found in directory: %s\n", dir_path);
        closedir(dir_stream);
        return 0; // Not an error, just no work to do
    }
    
    queue->filenames = calloc(queue->file_count, sizeof(char *));
    if (queue->filenames == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for filenames.\n");
        closedir(dir_stream);
        close(queue->dir_fd);
        queue->dir_fd = -1;
        return -1;
    }

    rewinddir(dir_stream);
    int i = 0;

    // Second pass: populate the filenames array
    while (i < queue->file_count && (entry = readdir(dir_stream)) != NULL) {
        int is_regular = 0;
        if (entry->d_type == DT_REG) {
            is_regular = 1;
        } else if (entry->d_type == DT_UNKNOWN) {
            struct stat st;
            if (fstatat(queue->dir_fd, entry->d_name, &st, AT_NO_AUTOMOUNT) == 0) {
                if (S_ISREG(st.st_mode)) {
                    is_regular = 1;
                }
            }
        }
        
        if (is_regular) {
            queue->filenames[i] = strdup(entry->d_name);
            if (queue->filenames[i] == NULL) {
                fprintf(stderr, "Error: strdup failed for filename.\n");
                // Cleanup already allocated names
                for (int j = 0; j < i; j++) {
                    free(queue->filenames[j]);
                }
                free(queue->filenames);
                queue->filenames = NULL;
                closedir(dir_stream);
                close(queue->dir_fd);
                queue->dir_fd = -1;
                return -1;
            }
            i++;
        }
    }
    
    // In case the directory content changed between passes
    queue->file_count = i;

    closedir(dir_stream);
    return 0;
}

/**
 * @brief Frees resources associated with the work queue.
 *
 * @param queue The WorkQueue to clean up.
 */
void cleanup_work_queue(WorkQueue *queue) {
    if (queue == NULL) return;
    
    if (queue->filenames != NULL) {
        for (int i = 0; i < queue->file_count; i++) {
            free(queue->filenames[i]);
        }
        free(queue->filenames);
        queue->filenames = NULL;
    }
    
    if(queue->dir_fd != -1) {
        close(queue->dir_fd);
        queue->dir_fd = -1;
    }
    pthread_mutex_destroy(&queue->mutex);
}

/**
 * @brief Main logic for setting up, running, and tearing down the concurrent processing.
 *
 * @param test_name A descriptive name for the test case.
 * @param dir_path The directory to process.
 */
void run_test(const char *test_name, const char *dir_path) {
    printf("\n--- Running Test Case: %s ---\n", test_name);
    printf("Target Directory: %s\n", dir_path);

    WorkQueue queue = {0};
    queue.next_file_index = 0;
    queue.dir_fd = -1;

    if (populate_work_queue(&queue, dir_path) != 0) {
        // Error message already printed in populate_work_queue
        return;
    }

    if (queue.file_count == 0) {
        cleanup_work_queue(&queue);
        return;
    }

    if (pthread_mutex_init(&queue.mutex, NULL) != 0) {
        fprintf(stderr, "Error: Mutex initialization failed.\n");
        cleanup_work_queue(&queue);
        return;
    }

    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i + 1;
        thread_args[i].queue = &queue;
        if (pthread_create(&threads[i], NULL, worker_thread, &thread_args[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i + 1);
            // In a real app, would need to gracefully stop already created threads
            queue.file_count = 0; // Prevent other threads from running
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (threads[i] != 0) {
           pthread_join(threads[i], NULL);
        }
    }

    printf("All threads finished.\n");
    cleanup_work_queue(&queue);
}

/**
 * @brief Creates a test directory and populates it with dummy files/subdirs.
 * @return 0 on success, -1 on failure.
 */
int setup_test_directory(const char* dir_path) {
    if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }
    
    char filepath[512];
    for (int i = 0; i < 5; i++) {
        snprintf(filepath, sizeof(filepath), "%s/file%d.txt", dir_path, i);
        FILE* f = fopen(filepath, "w");
        if (f) {
            fprintf(f, "This is test file %d.\n", i);
            fclose(f);
        }
    }
    
    snprintf(filepath, sizeof(filepath), "%s/subdir", dir_path);
    if (mkdir(filepath, 0755) == -1 && errno != EEXIST) {
        perror("mkdir subdir");
    }

    snprintf(filepath, sizeof(filepath), "%s/symlink_to_file0", dir_path);
    if (symlink("file0.txt", filepath) == -1 && errno != EEXIST) {
        perror("symlink");
    }

    return 0;
}

/**
 * @brief Cleans up (removes) the test directory and its contents.
 */
void cleanup_test_directory(const char* dir_path) {
    char command[512];
    snprintf(command, sizeof(command), "rm -rf %s", dir_path);
    if (system(command) != 0) {
        fprintf(stderr, "Warning: Failed to cleanup test directory %s\n", dir_path);
    }
}

int main(void) {
    // Test Case 1: Standard directory with files
    const char* test_dir_1 = "test_dir_1";
    setup_test_directory(test_dir_1);
    run_test("Standard Directory", test_dir_1);
    cleanup_test_directory(test_dir_1);

    // Test Case 2: Empty directory
    const char* test_dir_2 = "test_dir_2";
    if (mkdir(test_dir_2, 0755) == 0 || errno == EEXIST) {
        run_test("Empty Directory", test_dir_2);
        cleanup_test_directory(test_dir_2);
    }

    // Test Case 3: Directory with only a subdirectory (should be ignored)
    const char* test_dir_3 = "test_dir_3";
    char subdir_path[512];
    if (mkdir(test_dir_3, 0755) == 0 || errno == EEXIST) {
        snprintf(subdir_path, sizeof(subdir_path), "%s/subdir_only", test_dir_3);
        if (mkdir(subdir_path, 0755) == 0 || errno == EEXIST) {
            run_test("Directory with Subdirectory", test_dir_3);
        }
        cleanup_test_directory(test_dir_3);
    }

    // Test Case 4: Non-existent directory
    run_test("Non-existent Directory", "non_existent_dir");

    // Test Case 5: Path is a file, not a directory
    const char* test_file_5 = "test_file_5.txt";
    FILE* f = fopen(test_file_5, "w");
    if (f) {
        fclose(f);
        run_test("Path is a File", test_file_5);
        remove(test_file_5);
    }

    return 0;
}