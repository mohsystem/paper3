#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// --- Hash Set for Storing Used Names ---
// NOTE: This implementation is not thread-safe. For a multi-threaded
// application, mutexes (e.g., from pthreads) would be required.
#define HASH_TABLE_SIZE 10000
#define NAME_LENGTH 5
#define MAX_NAMES (26 * 26 * 1000)

typedef struct Node {
    char name[NAME_LENGTH + 1];
    struct Node* next;
} Node;

static Node* name_table[HASH_TABLE_SIZE] = {NULL};
static int name_count = 0;

// Simple hash function (djb2)
static unsigned int hash(const char* name) {
    unsigned long hash_value = 5381;
    int c;
    while ((c = *name++)) {
        hash_value = ((hash_value << 5) + hash_value) + c;
    }
    return hash_value % HASH_TABLE_SIZE;
}

// Check if a name exists in the set
static bool name_exists(const char* name) {
    unsigned int index = hash(name);
    Node* current = name_table[index];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Add a name to the set. Returns false if it already exists or on error.
static bool add_name(const char* name) {
    if (name_exists(name)) {
        return false;
    }
    unsigned int index = hash(name);
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return false;

    strncpy(new_node->name, name, NAME_LENGTH + 1);
    new_node->next = name_table[index];
    name_table[index] = new_node;
    name_count++;
    return true;
}

// Remove a name from the set
static void release_name(const char* name) {
    if (name == NULL || name[0] == '\0') return;

    unsigned int index = hash(name);
    Node* current = name_table[index];
    Node* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (prev == NULL) {
                name_table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            name_count--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

// --- Robot Name Generation ---

// Generates a random name, not guaranteed to be unique
static void generate_random_name(char* buffer) {
    // NOTE: rand() is not cryptographically secure. For a secure application,
    // a better source of randomness (like /dev/urandom on Linux) should be used.
    buffer[0] = 'A' + (rand() % 26);
    buffer[1] = 'A' + (rand() % 26);
    buffer[2] = '0' + (rand() % 10);
    buffer[3] = '0' + (rand() % 10);
    buffer[4] = '0' + (rand() % 10);
    buffer[5] = '\0';
}

// Generates a unique name and stores it in the output buffer.
bool generate_unique_name(char* out_name) {
    if (name_count >= MAX_NAMES) {
        return false; // All possible names are in use
    }
    do {
        generate_random_name(out_name);
    } while (!add_name(out_name)); // Loop until a unique name is added
    return true;
}

// --- Robot "Struct" Implementation ---

typedef struct {
    char name[NAME_LENGTH + 1];
} Robot;

Robot* create_robot() {
    Robot* robot = (Robot*)malloc(sizeof(Robot));
    if (robot) {
        robot->name[0] = '\0'; // No name initially
    }
    return robot;
}

const char* get_robot_name(Robot* robot) {
    if (robot->name[0] == '\0') {
        if (!generate_unique_name(robot->name)) {
            strcpy(robot->name, "ERROR"); 
        }
    }
    return robot->name;
}

void reset_robot(Robot* robot) {
    release_name(robot->name);
    robot->name[0] = '\0';
}

void destroy_robot(Robot* robot) {
    if (robot) {
        release_name(robot->name);
        free(robot);
    }
}

// Function to free all memory used by the name registry
void cleanup_name_registry() {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        Node* current = name_table[i];
        while (current != NULL) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
        name_table[i] = NULL;
    }
}

// --- Main function for testing ---
int main() {
    // Seed the random number generator once
    srand((unsigned int)time(NULL));

    printf("--- C Test Cases ---\n");

    // Test Case 1: Create a robot and get its name
    Robot* robot1 = create_robot();
    printf("Robot 1 initial name: %s\n", get_robot_name(robot1));

    // Test Case 2: Create another robot and get its name
    Robot* robot2 = create_robot();
    printf("Robot 2 initial name: %s\n", get_robot_name(robot2));

    // Test Case 3: Reset the first robot and get a new name
    printf("Resetting Robot 1...\n");
    char oldName1[NAME_LENGTH + 1];
    strcpy(oldName1, get_robot_name(robot1));
    reset_robot(robot1);
    printf("Robot 1 old name was: %s\n", oldName1);
    printf("Robot 1 new name is: %s\n", get_robot_name(robot1));

    // Test Case 4: Create more robots
    Robot* robot3 = create_robot();
    Robot* robot4 = create_robot();
    printf("Robot 3 name: %s\n", get_robot_name(robot3));
    printf("Robot 4 name: %s\n", get_robot_name(robot4));

    // Test Case 5: Get the name again, should be the same
    printf("Robot 4 name (again): %s\n", get_robot_name(robot4));
    
    // Cleanup
    destroy_robot(robot1);
    destroy_robot(robot2);
    destroy_robot(robot3);
    destroy_robot(robot4);
    
    cleanup_name_registry();

    printf("--------------------\n");
    return 0;
}