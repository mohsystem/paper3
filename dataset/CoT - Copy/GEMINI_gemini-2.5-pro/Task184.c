#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Global Name Registry ---
// Using a dynamic array to store used names
char** usedNames = NULL;
size_t nameCount = 0;
size_t nameCapacity = 0;

void initialize_registry() {
    nameCapacity = 10;
    usedNames = (char**)malloc(nameCapacity * sizeof(char*));
    if (usedNames == NULL) {
        fprintf(stderr, "Failed to allocate memory for name registry.\n");
        exit(1);
    }
    nameCount = 0;
}

void cleanup_registry() {
    for (size_t i = 0; i < nameCount; i++) {
        free(usedNames[i]);
    }
    free(usedNames);
    usedNames = NULL;
    nameCount = 0;
    nameCapacity = 0;
}

int is_name_used(const char* name) {
    for (size_t i = 0; i < nameCount; i++) {
        if (strcmp(usedNames[i], name) == 0) {
            return 1; // True, name is used
        }
    }
    return 0; // False, name is not used
}

void add_name_to_registry(const char* name) {
    if (nameCount >= nameCapacity) {
        nameCapacity *= 2;
        char** new_ptr = (char**)realloc(usedNames, nameCapacity * sizeof(char*));
        if (new_ptr == NULL) {
             fprintf(stderr, "Failed to reallocate memory for name registry.\n");
             // In a real app, you might try to recover, but here we exit.
             cleanup_registry();
             exit(1);
        }
        usedNames = new_ptr;
    }
    usedNames[nameCount] = (char*)malloc(6 * sizeof(char));
    strcpy(usedNames[nameCount], name);
    nameCount++;
}

void remove_name_from_registry(const char* name) {
    for (size_t i = 0; i < nameCount; i++) {
        if (strcmp(usedNames[i], name) == 0) {
            free(usedNames[i]);
            // Replace the found name with the last name in the list
            usedNames[i] = usedNames[nameCount - 1];
            nameCount--;
            return;
        }
    }
}

// --- Robot Definition ---
typedef struct {
    char* name;
} Robot;

void generate_random_name(char* buffer) {
    buffer[0] = 'A' + (rand() % 26);
    buffer[1] = 'A' + (rand() % 26);
    buffer[2] = '0' + (rand() % 10);
    buffer[3] = '0' + (rand() % 10);
    buffer[4] = '0' + (rand() % 10);
    buffer[5] = '\0';
}

char* generate_unique_name() {
    char newNameBuffer[6];
    do {
        generate_random_name(newNameBuffer);
    } while (is_name_used(newNameBuffer));

    add_name_to_registry(newNameBuffer);
    
    char* finalName = (char*)malloc(6 * sizeof(char));
    strcpy(finalName, newNameBuffer);
    return finalName;
}

// --- Robot Functions ---
Robot* create_robot() {
    Robot* robot = (Robot*)malloc(sizeof(Robot));
    if (robot == NULL) {
        return NULL;
    }
    robot->name = NULL;
    return robot;
}

void destroy_robot(Robot* robot) {
    if (robot != NULL) {
        free(robot->name); // robot->name could be NULL, free(NULL) is safe
        free(robot);
    }
}

const char* robot_get_name(Robot* robot) {
    if (robot->name == NULL) {
        robot->name = generate_unique_name();
    }
    return robot->name;
}

void robot_reset(Robot* robot) {
    if (robot->name != NULL) {
        remove_name_from_registry(robot->name);
        free(robot->name);
        robot->name = NULL;
    }
}


// --- Main Program ---
void Task184_main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    initialize_registry();

    // Test Case 1: Create a robot and get its name
    printf("--- Test Case 1 ---\n");
    Robot* r1 = create_robot();
    printf("Robot 1 initial name: %s\n", robot_get_name(r1));
    printf("Robot 1 name again: %s\n", robot_get_name(r1));

    // Test Case 2: Create another robot
    printf("\n--- Test Case 2 ---\n");
    Robot* r2 = create_robot();
    printf("Robot 2 initial name: %s\n", robot_get_name(r2));

    // Test Case 3: Reset the first robot and get a new name
    printf("\n--- Test Case 3 ---\n");
    printf("Resetting Robot 1...\n");
    robot_reset(r1);
    printf("Robot 1 new name: %s\n", robot_get_name(r1));

    // Test Case 4: Create a few more robots
    printf("\n--- Test Case 4 ---\n");
    Robot* r3 = create_robot();
    Robot* r4 = create_robot();
    printf("Robot 3 name: %s\n", robot_get_name(r3));
    printf("Robot 4 name: %s\n", robot_get_name(r4));

    // Test Case 5: Reset r2 and r3
    printf("\n--- Test Case 5 ---\n");
    printf("Resetting Robot 2 and 3...\n");
    robot_reset(r2);
    robot_reset(r3);
    printf("Robot 2 new name: %s\n", robot_get_name(r2));
    printf("Robot 3 new name: %s\n", robot_get_name(r3));
    printf("Robot 1 current name: %s\n", robot_get_name(r1));
    printf("Robot 4 current name: %s\n", robot_get_name(r4));

    // Cleanup
    destroy_robot(r1);
    destroy_robot(r2);
    destroy_robot(r3);
    destroy_robot(r4);
    cleanup_registry();
}

int main() {
    Task184_main();
    return 0;
}