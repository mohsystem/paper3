#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NAME_LEN 6 // 2 letters, 3 digits, 1 null terminator

// Global state for managing used names
static char** used_names = NULL;
static size_t used_names_count = 0;
static size_t used_names_capacity = 0;

typedef struct {
    char name[NAME_LEN];
} Robot;

// Forward declarations
void generate_random_name(char* buffer);
int is_name_used(const char* name);
void add_used_name(const char* name);
void remove_used_name(const char* name);
void cleanup_names();

void robot_init(Robot* robot);
const char* robot_get_name(const Robot* robot);
void robot_reset(Robot* robot);


// Generates a random name in the format "LLDDD"
void generate_random_name(char* buffer) {
    buffer[0] = 'A' + (rand() % 26);
    buffer[1] = 'A' + (rand() % 26);
    int num = rand() % 1000;
    sprintf(buffer + 2, "%03d", num);
}

// Checks if a name is already in the global list
int is_name_used(const char* name) {
    for (size_t i = 0; i < used_names_count; ++i) {
        if (strcmp(used_names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

// Adds a name to the global list, expanding it if necessary
void add_used_name(const char* name) {
    if (used_names_count >= used_names_capacity) {
        size_t new_capacity = (used_names_capacity == 0) ? 10 : used_names_capacity * 2;
        char** new_list = (char**)realloc(used_names, new_capacity * sizeof(char*));
        if (!new_list) {
            fprintf(stderr, "Failed to allocate memory for names.\n");
            exit(1);
        }
        used_names = new_list;
        used_names_capacity = new_capacity;
    }
    used_names[used_names_count] = (char*)malloc(NAME_LEN * sizeof(char));
    strcpy(used_names[used_names_count], name);
    used_names_count++;
}

// Removes a name from the global list
void remove_used_name(const char* name) {
    for (size_t i = 0; i < used_names_count; ++i) {
        if (strcmp(used_names[i], name) == 0) {
            free(used_names[i]);
            // Replace the found element with the last one
            used_names[i] = used_names[used_names_count - 1];
            used_names_count--;
            return;
        }
    }
}

// Frees all memory used by the global name list
void cleanup_names() {
    for (size_t i = 0; i < used_names_count; ++i) {
        free(used_names[i]);
    }
    free(used_names);
    used_names = NULL;
    used_names_count = 0;
    used_names_capacity = 0;
}

// Initializes a robot with a new unique name
void robot_init(Robot* robot) {
    char new_name[NAME_LEN];
    do {
        generate_random_name(new_name);
    } while (is_name_used(new_name));
    
    strcpy(robot->name, new_name);
    add_used_name(robot->name);
}

// Resets a robot, giving it a new unique name
void robot_reset(Robot* robot) {
    remove_used_name(robot->name);
    robot_init(robot);
}

// Returns the name of the robot
const char* robot_get_name(const Robot* robot) {
    return robot->name;
}

int main() {
    srand(time(NULL));

    printf("--- Test Case 1: Create 5 robots ---\n");
    Robot r1, r2, r3, r4, r5;
    robot_init(&r1);
    robot_init(&r2);
    robot_init(&r3);
    robot_init(&r4);
    robot_init(&r5);

    printf("Robot 1 name: %s\n", robot_get_name(&r1));
    printf("Robot 2 name: %s\n", robot_get_name(&r2));
    printf("Robot 3 name: %s\n", robot_get_name(&r3));
    printf("Robot 4 name: %s\n", robot_get_name(&r4));
    printf("Robot 5 name: %s\n", robot_get_name(&r5));
    
    printf("\n--- Test Case 2: Reset Robot 3 ---\n");
    char old_name_r3[NAME_LEN];
    strcpy(old_name_r3, robot_get_name(&r3));
    robot_reset(&r3);
    printf("Robot 3 old name: %s\n", old_name_r3);
    printf("Robot 3 new name: %s\n", robot_get_name(&r3));
    
    printf("\n--- Test Case 3: Reset Robot 1 ---\n");
    char old_name_r1[NAME_LEN];
    strcpy(old_name_r1, robot_get_name(&r1));
    robot_reset(&r1);
    printf("Robot 1 old name: %s\n", old_name_r1);
    printf("Robot 1 new name: %s\n", robot_get_name(&r1));

    printf("\n--- Test Case 4: Show current names ---\n");
    printf("Robot 1 name: %s\n", robot_get_name(&r1));
    printf("Robot 2 name: %s\n", robot_get_name(&r2));
    printf("Robot 3 name: %s\n", robot_get_name(&r3));
    printf("Robot 4 name: %s\n", robot_get_name(&r4));
    printf("Robot 5 name: %s\n", robot_get_name(&r5));
    
    printf("\n--- Test Case 5: Create a new robot ---\n");
    Robot r6;
    robot_init(&r6);
    printf("New Robot 6 name: %s\n", robot_get_name(&r6));
    printf("Total unique names in use: %zu\n", used_names_count);

    cleanup_names();
    return 0;
}