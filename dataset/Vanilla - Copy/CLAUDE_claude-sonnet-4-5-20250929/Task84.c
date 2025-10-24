
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define SESSION_ID_LENGTH 32
#define MAX_SESSIONS 1000

typedef struct {
    char sessions[MAX_SESSIONS][SESSION_ID_LENGTH + 1];
    int count;
} SessionManager;

static SessionManager manager = {.count = 0};

int isUnique(const char* sessionId) {
    for (int i = 0; i < manager.count; i++) {
        if (strcmp(manager.sessions[i], sessionId) == 0) {
            return 0;
        }
    }
    return 1;
}

void generateSessionId(char* sessionId) {
    int charactersLen = strlen(CHARACTERS);
    
    do {
        for (int i = 0; i < SESSION_ID_LENGTH; i++) {
            int index = rand() % charactersLen;
            sessionId[i] = CHARACTERS[index];
        }
        sessionId[SESSION_ID_LENGTH] = '\\0';
    } while (!isUnique(sessionId));
    
    if (manager.count < MAX_SESSIONS) {
        strcpy(manager.sessions[manager.count], sessionId);
        manager.count++;
    }
}

int main() {
    srand(time(NULL));
    
    char sessionId[SESSION_ID_LENGTH + 1];
    
    generateSessionId(sessionId);
    printf("Test Case 1: %s\\n", sessionId);
    
    generateSessionId(sessionId);
    printf("Test Case 2: %s\\n", sessionId);
    
    generateSessionId(sessionId);
    printf("Test Case 3: %s\\n", sessionId);
    
    generateSessionId(sessionId);
    printf("Test Case 4: %s\\n", sessionId);
    
    generateSessionId(sessionId);
    printf("Test Case 5: %s\\n", sessionId);
    
    return 0;
}
