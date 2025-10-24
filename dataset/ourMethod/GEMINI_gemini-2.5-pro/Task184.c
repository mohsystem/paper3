#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// Simple dynamic array to store used names
static char** used_names = NULL;
static size_t used_names_count = 0;
static size_t used_names_capacity = 0;
static pthread_mutex_t name_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char* name;
} Robot;

// Forward declaration
char* generate_unique_name();

Robot* robot_create() {
    Robot* robot = (Robot*)malloc(sizeof(Robot));
    if (robot == NULL) {
        perror("Failed to allocate memory for Robot");
        return NULL;
    }
    robot->name = NULL;
    return robot;
}

void robot_destroy(Robot* robot) {
    if (robot != NULL) {
        free(robot->name); // free(NULL) is safe
        free(robot);
    }
}

const char* robot_get_name(Robot* robot) {
    if (robot == NULL) {
        return "Invalid Robot";
    }
    if (robot->name == NULL) {
        robot->name = generate_unique_name();
    }
    return robot->name;
}

void robot_reset(Robot* robot) {
    if (robot == NULL || robot->name == NULL) {
        return;
    }

    pthread_mutex_lock(&name_mutex);
    for (size_t i = 0; i < used_names_count; ++i) {
        if (strcmp(used_names[i], robot->name) == 0) {
            free(used_names[i]);
            // Shift elements to fill the gap
            if (i < used_names_count - 1) {
                used_names[i] = used_names[used_names_count - 1];
            }
            used_names_count--;
            break;
        }
    }
    pthread_mutex_unlock(&name_mutex);

    free(robot->name);
    robot->name = NULL;
}

void cleanup_name_registry() {
    pthread_mutex_lock(&name_mutex);
    for (size_t i = 0; i < used_names_count; ++i) {
        free(used_names[i]);
    }
    free(used_names);
    used_names = NULL;
    used_names_count = 0;
    used_names_capacity = 0;
    pthread_mutex_unlock(&name_mutex);
    pthread_mutex_destroy(&name_mutex);
}

bool is_name_used(const char* name) {
    for (size_t i = 0; i < used_names_count; ++i) {
        if (strcmp(used_names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

void add_name_to_registry(const char* name) {
    if (used_names_count >= used_names_capacity) {
        size_t new_capacity = (used_names_capacity == 0) ? 10 : used_names_capacity * 2;
        char** new_names = (char**)realloc(used_names, new_capacity * sizeof(char*));
        if (new_names == NULL) {
            perror("Failed to reallocate name registry");
            // In a real app, handle this more gracefully
            return;
        }
        used_names = new_names;
        used_names_capacity = new_capacity;
    }
    used_names[used_names_count] = strdup(name);
    if(used_names[used_names_count] == NULL) {
         perror("Failed to duplicate string");
         return;
    }
    used_names_count++;
}

// Generates a random name and ensures it's unique
char* generate_unique_name() {
    char new_name[6]; // 2 letters, 3 digits, 1 null terminator
    unsigned char random_bytes[3];

    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd == -1) {
        perror("Error opening /dev/urandom");
        return NULL;
    }

    while (true) {
        ssize_t bytes_read = read(urandom_fd, random_bytes, sizeof(random_bytes));
        if (bytes_read < (ssize_t)sizeof(random_bytes)) {
             perror("Error reading from /dev/urandom");
             close(urandom_fd);
             return NULL;
        }

        char letter1 = 'A' + (random_bytes[0] % 26);
        char letter2 = 'A' + (random_bytes[1] % 26);
        int number = ((unsigned int)random_bytes[2] * 1000) / 256; // Scale to 0-999
        snprintf(new_name, sizeof(new_name), "%c%c%03d", letter1, letter2, number);

        pthread_mutex_lock(&name_mutex);
        if (!is_name_used(new_name)) {
            add_name_to_registry(new_name);
            pthread_mutex_unlock(&name_mutex);
            break;
        }
        pthread_mutex_unlock(&name_mutex);
    }

    close(urandom_fd);
    char* result = strdup(new_name);
    if(result == NULL) {
        perror("Failed to duplicate final name string");
    }
    return result;
}


int main() {
    // Test Case 1: Create a robot and get its name
    Robot* robot1 = robot_create();
    const char* name1 = robot_get_name(robot1);
    printf("Test Case 1: Robot 1's first name: %s\n", name1);

    // Test Case 2: Get the same robot's name again, should be the same
    const char* name1_again = robot_get_name(robot1);
    printf("Test Case 2: Robot 1's name again: %s\n", name1_again);
    printf("Name is consistent: %s\n", strcmp(name1, name1_again) == 0 ? "true" : "false");

    // Test Case 3: Create a second robot, its name should be different
    Robot* robot2 = robot_create();
    const char* name2 = robot_get_name(robot2);
    printf("Test Case 3: Robot 2's name: %s\n", name2);
    printf("Names are unique: %s\n", strcmp(name1, name2) != 0 ? "true" : "false");

    // Test Case 4: Reset the first robot and get its new name
    printf("Test Case 4: Resetting Robot 1...\n");
    robot_reset(robot1);
    const char* new_name1 = robot_get_name(robot1);
    printf("Robot 1's new name: %s\n", new_name1);
    printf("New name is different from old name: %s\n", strcmp(name1, new_name1) != 0 ? "true" : "false");
    printf("New name is unique from Robot 2: %s\n", strcmp(new_name1, name2) != 0 ? "true" : "false");
    
    // Test Case 5: Create more robots
    printf("Test Case 5: Creating more robots...\n");
    Robot* robot3 = robot_create();
    Robot* robot4 = robot_create();
    printf("Robot 3's name: %s\n", robot_get_name(robot3));
    printf("Robot 4's name: %s\n", robot_get_name(robot4));
    
    // Cleanup
    robot_destroy(robot1);
    robot_destroy(robot2);
    robot_destroy(robot3);
    robot_destroy(robot4);
    cleanup_name_registry();

    return 0;
}