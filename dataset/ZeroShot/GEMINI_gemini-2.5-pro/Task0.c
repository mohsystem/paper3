#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// For cross-platform compatibility (mkdir)
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#define MKDIR(path, mode) mkdir(path, mode)
#endif


// Node for the linked list queue
typedef struct Node {
    char* file_path;
    struct Node* next;
} Node;

// Thread-safe queue
typedef struct {
    Node* head;
    Node* tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int is_done; // Flag to signal workers that no more items will be added
} SafeQueue;

void init_queue(SafeQueue* q) {
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    q->is_done = 0;
}

void destroy_queue(SafeQueue* q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
    // Free any remaining nodes
    Node* current = q->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp->file_path);
        free(temp);
    }
}

void enqueue(SafeQueue* q, const char* file_path) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        perror("Failed to allocate memory for queue node");
        return;
    }
    new_node->file_path = strdup(file_path);
    if (!new_node->file_path) {
        perror("Failed to duplicate file path string");
        free(new_node);
        return;
    }
    new_node->next = NULL;

    pthread_mutex_lock(&q->mutex);
    if (q->tail == NULL) {
        q->head = q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    pthread_cond_signal(&q->cond); // Signal one waiting thread
    pthread_mutex_unlock(&q->mutex);
}

// Dequeues a file path. Returns NULL if queue is empty and is_done is true.
char* dequeue(SafeQueue* q) {
    pthread_mutex_lock(&q->mutex);
    while (q->head == NULL) {
        if (q->is_done) {
            pthread_mutex_unlock(&q->mutex);
            return NULL; // No more work
        }
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    Node* temp = q->head;
    char* file_path = temp->file_path;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(temp);
    pthread_mutex_unlock(&q->mutex);
    return file_path;
}

void process_file(const char* file_path) {
    pthread_t tid = pthread_self();
    // Using basename might not be portable, so let's find the last '/' or '\'
    const char *filename = strrchr(file_path, '/');
    if (!filename) {
        filename = strrchr(file_path, '\\');
    }
    filename = (filename) ? filename + 1 : file_path;

    printf("Thread %ld started processing: %s\n", (long)tid, filename);
    fflush(stdout);
    
    // Simulate work
    usleep((50 + rand() % 50) * 1000);

    printf("Thread %ld finished processing: %s\n", (long)tid, filename);
    fflush(stdout);
}

void* worker(void* arg) {
    SafeQueue* q = (SafeQueue*)arg;
    while (1) {
        char* file_path = dequeue(q);
        if (file_path == NULL) {
            break; // No more work
        }
        process_file(file_path);
        free(file_path); // Free the string allocated by strdup
    }
    return NULL;
}

void cleanup_directory(const char *path) {
    DIR *d = opendir(path);
    if (!d) return;
    
    struct dirent *p;
    while ((p = readdir(d)) != NULL) {
        if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, p->d_name);
        remove(full_path);
    }
    closedir(d);
    rmdir(path);
}

void run_test(const char* directory_path, int num_files, int num_threads) {
    printf("\n--- Running Test ---\n");
    printf("Directory: %s, Files: %d, Threads: %d\n", directory_path, num_files, num_threads);

    // 1. Setup: Create directory and dummy files
    if (MKDIR(directory_path, 0755) != 0 && errno != EEXIST) {
        perror("Failed to create directory");
        return;
    }
    for (int i = 1; i <= num_files; ++i) {
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s/file_%d.txt", directory_path, i);
        FILE *f = fopen(file_path, "w");
        if (f) fclose(f);
    }
    
    // 2. Initialize and populate queue
    SafeQueue q;
    init_queue(&q);
    
    DIR* d = opendir(directory_path);
    if (d) {
        struct dirent* dir;
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, dir->d_name);
                enqueue(&q, full_path);
            }
        }
        closedir(d);
    }
    
    // 3. Create and start threads
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], NULL, worker, &q);
    }
    
    // Signal that all items are enqueued
    pthread_mutex_lock(&q.mutex);
    q.is_done = 1;
    pthread_cond_broadcast(&q.cond); // Wake up all waiting threads
    pthread_mutex_unlock(&q.mutex);
    
    // 4. Wait for threads to finish
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    
    // 5. Cleanup
    destroy_queue(&q);
    cleanup_directory(directory_path);
    printf("Cleaned up directory: %s\n", directory_path);
    printf("--- Test Finished ---\n");
}

int main() {
    // Test Case 1: More files than threads
    run_test("c_test_dir_1", 5, 2);
    // Test Case 2: Equal files and threads
    run_test("c_test_dir_2", 4, 4);
    // Test Case 3: More threads than files
    run_test("c_test_dir_3", 3, 5);
    // Test Case 4: Single thread
    run_test("c_test_dir_4", 5, 1);
    // Test Case 5: Large number of files
    run_test("c_test_dir_5", 20, 4);
    
    return 0;
}