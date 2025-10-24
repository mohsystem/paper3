#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // For sleep()

// Shared data structure for threads
typedef struct {
    char** files;
    int num_files;
    int* current_index;
    pthread_mutex_t* mutex;
    int thread_id;
} thread_data_t;

void* file_processor(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    
    while (1) {
        char* file_to_process = NULL;
        int file_index = -1;

        // Lock mutex to safely get the next file index
        pthread_mutex_lock(data->mutex);

        if (*(data->current_index) < data->num_files) {
            file_index = *(data->current_index);
            file_to_process = data->files[file_index];
            (*(data->current_index))++;
        }
        
        // Unlock mutex as soon as we are done with shared data
        pthread_mutex_unlock(data->mutex);

        if (file_to_process == NULL) {
            break; // No more files
        }

        printf("Thread-%d started processing %s\n", data->thread_id, file_to_process);
        
        // Simulate file processing time
        sleep(1);
        
        printf("Thread-%d finished processing %s\n", data->thread_id, file_to_process);
    }

    free(data); // Free the thread-specific data struct
    return NULL;
}

void processFiles(char* files[], int num_files, int num_threads) {
    printf("--- C Concurrent File Processing ---\n");
    if (num_files == 0) {
        printf("No files to process.\n");
        printf("All files have been processed.\n\n");
        return;
    }

    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Failed to allocate memory for threads");
        return;
    }
    
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    int current_index = 0;

    for (int i = 0; i < num_threads; ++i) {
        thread_data_t* data = (thread_data_t*)malloc(sizeof(thread_data_t));
        if (data == NULL) {
            perror("Failed to allocate memory for thread data");
            continue;
        }
        data->files = files;
        data->num_files = num_files;
        data->current_index = &current_index;
        data->mutex = &mutex;
        data->thread_id = i + 1;

        if (pthread_create(&threads[i], NULL, file_processor, data) != 0) {
            perror("Failed to create thread");
            free(data);
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        if (threads[i] != 0) {
            pthread_join(threads[i], NULL);
        }
    }

    pthread_mutex_destroy(&mutex);
    free(threads);
    
    printf("All files have been processed.\n\n");
}

int main() {
    // Test Case 1
    char* files1[] = {"file1.log", "file2.data", "archive.zip", "image.jpg", "report.docx"};
    processFiles(files1, 5, 3);
    
    // Test Case 2
    char* files2[] = {"document.pdf", "data.csv"};
    processFiles(files2, 2, 2);
    
    // Test Case 3
    char* files3[] = {"single_file.txt"};
    processFiles(files3, 1, 4);
    
    // Test Case 4
    char* files4[] = {"audio.mp3", "video.mp4", "backup.sql", "config.ini", "style.css", "script.js"};
    processFiles(files4, 6, 4);
    
    // Test Case 5
    processFiles(NULL, 0, 3); // No files

    return 0;
}