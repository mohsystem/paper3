
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_NAMES 676000
#define NAME_LENGTH 6

typedef struct {
    char name[NAME_LENGTH];
} Robot;

static char usedNames[MAX_NAMES][NAME_LENGTH];
static int usedNamesCount = 0;
static bool initialized = false;

void initRandom() {
    if (!initialized) {
        srand(time(NULL));
        initialized = true;
    }
}

bool isNameUsed(const char* name) {
    for (int i = 0; i < usedNamesCount; i++) {
        if (strcmp(usedNames[i], name) == 0) {
            return true;
        }
    }
    return false;
}

void generateRandomName(char* buffer) {
    char letter1 = 'A' + (rand() % 26);
    char letter2 = 'A' + (rand() % 26);
    int number = rand() % 1000;
    sprintf(buffer, "%c%c%03d", letter1, letter2, number);
}

void generateUniqueName(char* buffer) {
    if (usedNamesCount >= MAX_NAMES) {
        fprintf(stderr, "All possible robot names have been exhausted\\n");
        exit(1);
    }
    
    do {
        generateRandomName(buffer);
    } while (isNameUsed(buffer));
    
    strcpy(usedNames[usedNamesCount], buffer);
    usedNamesCount++;
}

Robot* createRobot() {
    initRandom();
    Robot* robot = (Robot*)malloc(sizeof(Robot));
    if (robot == NULL) {
        return NULL;
    }
    generateUniqueName(robot->name);
    return robot;
}

const char* getRobotName(Robot* robot) {
    return robot->name;
}

void resetRobot(Robot* robot) {
    for (int i = 0; i < usedNamesCount; i++) {
        if (strcmp(usedNames[i], robot->name) == 0) {
            for (int j = i; j < usedNamesCount - 1; j++) {
                strcpy(usedNames[j], usedNames[j + 1]);
            }
            usedNamesCount--;
            break;
        }
    }
    generateUniqueName(robot->name);
}

void destroyRobot(Robot* robot) {
    if (robot != NULL) {
        for (int i = 0; i < usedNamesCount; i++) {
            if (strcmp(usedNames[i], robot->name) == 0) {
                for (int j = i; j < usedNamesCount - 1; j++) {
                    strcpy(usedNames[j], usedNames[j + 1]);
                }
                usedNamesCount--;
                break;
            }
        }
        free(robot);
    }
}

int main() {
    printf("Test Case 1: Creating new robots\\n");
    Robot* robot1 = createRobot();
    Robot* robot2 = createRobot();
    printf("Robot 1: %s\\n", getRobotName(robot1));
    printf("Robot 2: %s\\n", getRobotName(robot2));
    printf("Names are unique: %s\\n", strcmp(getRobotName(robot1), getRobotName(robot2)) != 0 ? "true" : "false");
    
    printf("\\nTest Case 2: Resetting a robot\\n");
    char oldName[NAME_LENGTH];
    strcpy(oldName, getRobotName(robot1));
    resetRobot(robot1);
    printf("Old name: %s\\n", oldName);
    printf("New name: %s\\n", getRobotName(robot1));
    printf("Names are different: %s\\n", strcmp(oldName, getRobotName(robot1)) != 0 ? "true" : "false");
    
    printf("\\nTest Case 3: Creating multiple robots\\n");
    Robot* robots[5];
    for (int i = 0; i < 5; i++) {
        robots[i] = createRobot();
        printf("Robot: %s\\n", getRobotName(robots[i]));
    }
    
    printf("\\nTest Case 4: Resetting multiple robots\\n");
    for (int i = 0; i < 5; i++) {
        char before[NAME_LENGTH];
        strcpy(before, getRobotName(robots[i]));
        resetRobot(robots[i]);
        printf("%s -> %s\\n", before, getRobotName(robots[i]));
    }
    
    printf("\\nTest Case 5: Verify name format\\n");
    Robot* robot3 = createRobot();
    const char* testName = getRobotName(robot3);
    bool validFormat = strlen(testName) == 5 &&
                       testName[0] >= 'A' && testName[0] <= 'Z' &&
                       testName[1] >= 'A' && testName[1] <= 'Z' &&
                       testName[2] >= '0' && testName[2] <= '9' &&
                       testName[3] >= '0' && testName[3] <= '9' &&
                       testName[4] >= '0' && testName[4] <= '9';
    printf("Robot name: %s\\n", testName);
    printf("Valid format (2 letters + 3 digits): %s\\n", validFormat ? "true" : "false");
    
    destroyRobot(robot1);
    destroyRobot(robot2);
    destroyRobot(robot3);
    for (int i = 0; i < 5; i++) {
        destroyRobot(robots[i]);
    }
    
    return 0;
}
