
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_NAMES 676000
#define NAME_LENGTH 6

typedef struct {
    char names[MAX_NAMES][NAME_LENGTH];
    int count;
} NameRegistry;

typedef struct {
    char name[NAME_LENGTH];
} Task184;

static NameRegistry registry = {.count = 0};

static void generate_random_name(char* buffer) {
    buffer[0] = 'A' + (rand() % 26);
    buffer[1] = 'A' + (rand() % 26);
    int digits = rand() % 1000;
    snprintf(buffer + 2, 4, "%03d", digits);
    buffer[5] = '\\0';
}

static bool is_name_used(const char* name) {
    for (int i = 0; i < registry.count; i++) {
        if (strcmp(registry.names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

static void generate_unique_name(char* buffer) {
    if (registry.count >= MAX_NAMES) {
        fprintf(stderr, "All possible robot names have been exhausted\\n");
        exit(1);
    }
    
    do {
        generate_random_name(buffer);
    } while (is_name_used(buffer));
    
    strcpy(registry.names[registry.count], buffer);
    registry.count++;
}

static void remove_name(const char* name) {
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

void Task184_init(Task184* robot) {
    generate_unique_name(robot->name);
}

const char* Task184_get_name(const Task184* robot) {
    return robot->name;
}

void Task184_reset(Task184* robot) {
    remove_name(robot->name);
    generate_unique_name(robot->name);
}

bool matches_pattern(const char* name) {
    if (strlen(name) != 5) return false;
    if (name[0] < 'A' || name[0] > 'Z') return false;
    if (name[1] < 'A' || name[1] > 'Z') return false;
    if (name[2] < '0' || name[2] > '9') return false;
    if (name[3] < '0' || name[3] > '9') return false;
    if (name[4] < '0' || name[4] > '9') return false;
    return true;
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1: Create robot and get name\\n");
    Task184 robot1;
    Task184_init(&robot1);
    const char* name1 = Task184_get_name(&robot1);
    printf("Robot 1 name: %s\\n", name1);
    printf("Name matches pattern: %s\\n", matches_pattern(name1) ? "true" : "false");
    
    printf("\\nTest Case 2: Create another robot with different name\\n");
    Task184 robot2;
    Task184_init(&robot2);
    const char* name2 = Task184_get_name(&robot2);
    printf("Robot 2 name: %s\\n", name2);
    printf("Names are different: %s\\n", strcmp(name1, name2) != 0 ? "true" : "false");
    
    printf("\\nTest Case 3: Reset robot and verify new name\\n");
    char oldName[NAME_LENGTH];
    strcpy(oldName, Task184_get_name(&robot1));
    Task184_reset(&robot1);
    const char* newName = Task184_get_name(&robot1);
    printf("Old name: %s\\n", oldName);
    printf("New name: %s\\n", newName);
    printf("Names are different: %s\\n", strcmp(oldName, newName) != 0 ? "true" : "false");
    
    printf("\\nTest Case 4: Create multiple robots\\n");
    Task184 robot3, robot4, robot5;
    Task184_init(&robot3);
    Task184_init(&robot4);
    Task184_init(&robot5);
    printf("Robot 3: %s\\n", Task184_get_name(&robot3));
    printf("Robot 4: %s\\n", Task184_get_name(&robot4));
    printf("Robot 5: %s\\n", Task184_get_name(&robot5));
    
    printf("\\nTest Case 5: Verify uniqueness after resets\\n");
    Task184_reset(&robot2);
    Task184_reset(&robot3);
    const char* names[5] = {
        Task184_get_name(&robot1),
        Task184_get_name(&robot2),
        Task184_get_name(&robot3),
        Task184_get_name(&robot4),
        Task184_get_name(&robot5)
    };
    int uniqueCount = 5;
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (strcmp(names[i], names[j]) == 0) {
                uniqueCount--;
                break;
            }
        }
    }
    printf("All 5 names are unique: %s\\n", uniqueCount == 5 ? "true" : "false");
    
    return 0;
}
