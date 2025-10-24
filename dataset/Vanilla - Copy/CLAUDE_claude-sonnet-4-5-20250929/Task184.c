
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_ROBOTS 100000
#define NAME_LENGTH 6

typedef struct {
    char names[MAX_ROBOTS][NAME_LENGTH];
    int count;
} NameRegistry;

typedef struct {
    char name[NAME_LENGTH];
} Task184;

static NameRegistry registry = {{}, 0};
static bool seeded = false;

static void generateRandomName(char* name) {
    if (!seeded) {
        srand(time(NULL));
        seeded = true;
    }
    
    char letter1 = 'A' + (rand() % 26);
    char letter2 = 'A' + (rand() % 26);
    int number = rand() % 1000;
    
    sprintf(name, "%c%c%03d", letter1, letter2, number);
}

static bool isNameUsed(const char* name) {
    for (int i = 0; i < registry.count; i++) {
        if (strcmp(registry.names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

static void addName(const char* name) {
    if (registry.count < MAX_ROBOTS) {
        strcpy(registry.names[registry.count], name);
        registry.count++;
    }
}

static void removeName(const char* name) {
    for (int i = 0; i < registry.count; i++) {
        if (strcmp(registry.names[i], name) == 0) {
            for (int j = i; j < registry.count - 1; j++) {
                strcpy(registry.names[j], registry.names[j + 1]);
            }
            registry.count--;
            break;
        }
    }
}

static void generateUniqueName(char* name) {
    do {
        generateRandomName(name);
    } while (isNameUsed(name));
    addName(name);
}

void Task184_init(Task184* robot) {
    generateUniqueName(robot->name);
}

const char* Task184_getName(const Task184* robot) {
    return robot->name;
}

void Task184_reset(Task184* robot) {
    if (strlen(robot->name) > 0) {
        removeName(robot->name);
    }
    generateUniqueName(robot->name);
}

int main() {
    printf("Test Case 1: Create a robot and get its name\\n");
    Task184 robot1;
    Task184_init(&robot1);
    const char* name1 = Task184_getName(&robot1);
    printf("Robot 1 name: %s\\n", name1);
    printf("Name length is 5: %s\\n", strlen(name1) == 5 ? "true" : "false");
    
    printf("\\nTest Case 2: Create another robot with different name\\n");
    Task184 robot2;
    Task184_init(&robot2);
    const char* name2 = Task184_getName(&robot2);
    printf("Robot 2 name: %s\\n", name2);
    printf("Names are different: %s\\n", strcmp(name1, name2) != 0 ? "true" : "false");
    
    printf("\\nTest Case 3: Reset robot and verify new name\\n");
    char oldName[NAME_LENGTH];
    strcpy(oldName, Task184_getName(&robot1));
    Task184_reset(&robot1);
    const char* newName = Task184_getName(&robot1);
    printf("Old name: %s\\n", oldName);
    printf("New name: %s\\n", newName);
    printf("Names are different after reset: %s\\n", strcmp(oldName, newName) != 0 ? "true" : "false");
    
    printf("\\nTest Case 4: Create multiple robots and verify uniqueness\\n");
    Task184 robots[10];
    int uniqueCount = 0;
    for (int i = 0; i < 10; i++) {
        Task184_init(&robots[i]);
        bool isUnique = true;
        for (int j = 0; j < i; j++) {
            if (strcmp(Task184_getName(&robots[i]), Task184_getName(&robots[j])) == 0) {
                isUnique = false;
                break;
            }
        }
        if (isUnique) uniqueCount++;
    }
    printf("Created 10 robots, unique names count: %d\\n", uniqueCount);
    printf("All names are unique: %s\\n", uniqueCount == 10 ? "true" : "false");
    
    printf("\\nTest Case 5: Reset multiple times\\n");
    Task184 robot3;
    Task184_init(&robot3);
    char resetNames[6][NAME_LENGTH];
    strcpy(resetNames[0], Task184_getName(&robot3));
    int resetUniqueCount = 1;
    for (int i = 1; i < 6; i++) {
        Task184_reset(&robot3);
        strcpy(resetNames[i], Task184_getName(&robot3));
        bool isUnique = true;
        for (int j = 0; j < i; j++) {
            if (strcmp(resetNames[i], resetNames[j]) == 0) {
                isUnique = false;
                break;
            }
        }
        if (isUnique) resetUniqueCount++;
    }
    printf("Robot reset 5 times, unique names: %d\\n", resetUniqueCount);
    printf("All reset names are unique: %s\\n", resetUniqueCount == 6 ? "true" : "false");
    
    return 0;
}
