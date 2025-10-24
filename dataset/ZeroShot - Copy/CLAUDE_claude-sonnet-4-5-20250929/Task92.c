
#include <stdio.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    int current_counter;
    int max_counter;
    pthread_mutex_t lock;
} SharedResource;

typedef struct {
    SharedResource* resource;
    char thread_name[20];
    int iterations;
} ThreadArgs;

void access_shared_resource(SharedResource* resource, const char* thread_name) {
    if (resource->current_counter <= resource->max_counter) {
        pthread_mutex_lock(&resource->lock);
        if (resource->current_counter <= resource->max_counter) {
            resource->current_counter++;
            printf("%s is accessing currentCounter: %d\\n", thread_name, resource->current_counter);
        }
        pthread_mutex_unlock(&resource->lock);
    }
}

void* thread_function(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = 0; i < args->iterations; i++) {
        access_shared_resource(args->resource, args->thread_name);
    }
    return NULL;
}

void run_test(int test_num, int max_counter, int num_threads, int iterations) {
    printf("\\nTest Case %d:\\n", test_num);
    
    SharedResource resource;
    resource.current_counter = 0;
    resource.max_counter = max_counter;
    pthread_mutex_init(&resource.lock, NULL);

    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    for (int i = 0; i < num_threads; i++) {
        args[i].resource = &resource;
        sprintf(args[i].thread_name, "Thread-%d", i + 1);
        args[i].iterations = iterations;
        pthread_create(&threads[i], NULL, thread_function, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&resource.lock);
}

int main() {
    // Test case 1: 3 threads with maxCounter = 5
    run_test(1, 5, 3, 3);

    // Test case 2: 2 threads with maxCounter = 3
    run_test(2, 3, 2, 2);

    // Test case 3: 4 threads with maxCounter = 8
    run_test(3, 8, 4, 3);

    // Test case 4: 5 threads with maxCounter = 10
    run_test(4, 10, 5, 3);

    // Test case 5: 2 threads with maxCounter = 1
    run_test(5, 1, 2, 1);

    return 0;
}
