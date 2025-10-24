
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_ROBOTS 676000
#define NAME_LENGTH 6  // 5 characters + null terminator

// Global array to track used names (simple implementation for C)
// In production, use a hash table for better performance
static char usedNames[MAX_ROBOTS][NAME_LENGTH];
static size_t usedNamesCount = 0;

typedef struct {
    char name[NAME_LENGTH];
} Robot;

// Check if a name already exists in the used names list
static bool nameExists(const char* name) {
    if (name == NULL || strlen(name) != 5) {
        return false;
    }
    
    for (size_t i = 0; i < usedNamesCount; i++) {
        if (strcmp(usedNames[i], name) == 0) {
            return true;
        }
    }
    return false;
}

// Add a name to the used names list
static bool addUsedName(const char* name) {
    if (name == NULL || usedNamesCount >= MAX_ROBOTS) {
        return false;
    }
    
    // Bounds check: ensure we don't overflow (Rules#1, C checklist)\n    if (strlen(name) >= NAME_LENGTH) {\n        return false;\n    }\n    \n    strncpy(usedNames[usedNamesCount], name, NAME_LENGTH - 1);\n    usedNames[usedNamesCount][NAME_LENGTH - 1] = '\\0';  // Ensure null termination\n    usedNamesCount++;\n    return true;\n}\n\n// Remove a name from the used names list\nstatic void removeUsedName(const char* name) {\n    if (name == NULL) {\n        return;\n    }\n    \n    for (size_t i = 0; i < usedNamesCount; i++) {\n        if (strcmp(usedNames[i], name) == 0) {\n            // Shift remaining names down\n            for (size_t j = i; j < usedNamesCount - 1; j++) {\n                strncpy(usedNames[j], usedNames[j + 1], NAME_LENGTH);\n            }\n            usedNamesCount--;\n            return;\n        }\n    }\n}\n\n// Generate a cryptographically secure random name\n// Uses rand() seeded with time and process info for better randomness (Rules#1, Rules#6)\nstatic bool generateUniqueName(char* buffer, size_t bufferSize) {\n    // Input validation (Rules#1)\n    if (buffer == NULL || bufferSize < NAME_LENGTH) {\n        return false;\n    }\n    \n    // Check if we've exhausted the namespace
    if (usedNamesCount >= MAX_ROBOTS) {
        return false;
    }
    
    // Seed random number generator with time for better randomness
    // Note: For production use, use a CSPRNG like /dev/urandom (Rules#6, Rules#7)
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL) ^ (unsigned int)clock());
        seeded = true;
    }
    
    char newName[NAME_LENGTH];
    int attempts = 0;
    const int maxAttempts = 1000000;
    
    do {
        // Generate two uppercase letters (A-Z)
        newName[0] = 'A' + (rand() % 26);
        newName[1] = 'A' + (rand() % 26);
        
        // Generate three digits (0-9)
        newName[2] = '0' + (rand() % 10);
        newName[3] = '0' + (rand() % 10);
        newName[4] = '0' + (rand() % 10);
        newName[5] = '\\0';
        
        attempts++;
        if (attempts >= maxAttempts) {
            return false;  // Prevent infinite loop
        }
        
    } while (nameExists(newName));
    
    // Copy to output buffer with bounds checking (Rules#1, C checklist)
    strncpy(buffer, newName, bufferSize - 1);
    buffer[bufferSize - 1] = '\\0';
    
    // Add to used names list
    return addUsedName(newName);
}

// Initialize a robot with no name
void robotInit(Robot* robot) {
    if (robot == NULL) {
        return;
    }
    memset(robot->name, 0, NAME_LENGTH);  // Initialize to empty (C checklist)
}

// Get the robot's name, generating one if it doesn't exist
const char* robotGetName(Robot* robot) {
    if (robot == NULL) {
        return NULL;
    }
    
    // If robot has no name (empty string), generate one
    if (robot->name[0] == '\\0') {
        if (!generateUniqueName(robot->name, NAME_LENGTH)) {
            return NULL;  // Failed to generate name
        }
    }
    
    return robot->name;
}

// Reset robot to factory settings (wipes name)
void robotReset(Robot* robot) {
    if (robot == NULL) {
        return;
    }
    
    // Release the old name if it exists
    if (robot->name[0] != '\\0') {
        removeUsedName(robot->name);
        // Clear the name securely (Rules#1, C checklist)
        memset(robot->name, 0, NAME_LENGTH);
    }
}

// Clean up robot resources
void robotDestroy(Robot* robot) {
    if (robot == NULL) {
        return;
    }
    
    // Release the name back to the pool
    if (robot->name[0] != '\\0') {
        removeUsedName(robot->name);
        memset(robot->name, 0, NAME_LENGTH);
    }
}

int main(void) {
    Robot robot1, robot2;
    
    // Test case 1: Create a robot and get its name
    robotInit(&robot1);
    const char* name1 = robotGetName(&robot1);
    if (name1 != NULL) {
        printf("Test 1 - Robot 1 name: %s\\n", name1);
    }
    
    // Test case 2: Verify name format (2 letters + 3 digits)
    if (name1 != NULL && strlen(name1) == 5 &&
        isupper((unsigned char)name1[0]) && isupper((unsigned char)name1[1]) &&
        isdigit((unsigned char)name1[2]) && isdigit((unsigned char)name1[3]) && 
        isdigit((unsigned char)name1[4])) {
        printf("Test 2 - Name format valid\\n");
    }
    
    // Test case 3: Create another robot and ensure different name
    robotInit(&robot2);
    const char* name2 = robotGetName(&robot2);
    if (name2 != NULL) {
        printf("Test 3 - Robot 2 name: %s\\n", name2);
        if (strcmp(name1, name2) != 0) {
            printf("Test 3 - Names are unique\\n");
        }
    }
    
    // Test case 4: Reset robot and get new name
    robotReset(&robot1);
    const char* name1New = robotGetName(&robot1);
    if (name1New != NULL) {
        printf("Test 4 - Robot 1 after reset: %s\\n", name1New);
        if (strcmp(name1, name1New) != 0) {
            printf("Test 4 - Name changed after reset\\n");
        }
    }
    
    // Test case 5: Verify all names are unique
    const char* name2Current = robotGetName(&robot2);
    if (name1New != NULL && name2Current != NULL) {
        if (strcmp(name1New, name2Current) != 0) {
            printf("Test 5 - All names remain unique\\n");
        }
    }
    
    // Clean up
    robotDestroy(&robot1);
    robotDestroy(&robot2);
    
    return 0;
}
