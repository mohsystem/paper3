
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SESSION_ID_LENGTH 32
#define MAX_STORED_IDS 10000

static const char CHARACTERS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static char generatedIds[MAX_STORED_IDS][SESSION_ID_LENGTH + 1];
static int idCount = 0;
static int initialized = 0;

int isUnique(const char* sessionId) {
    for (int i = 0; i < idCount; i++) {
        if (strcmp(generatedIds[i], sessionId) == 0) {
            return 0;
        }
    }
    return 1;
}

char* generateSessionId(char* buffer) {
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    
    int charactersLen = strlen(CHARACTERS);
    
    do {
        for (int i = 0; i < SESSION_ID_LENGTH; i++) {
            buffer[i] = CHARACTERS[rand() % charactersLen];
        }
        buffer[SESSION_ID_LENGTH] = '\\0';
    } while (!isUnique(buffer));
    
    if (idCount < MAX_STORED_IDS) {
        strcpy(generatedIds[idCount], buffer);
        idCount++;
    }
    
    return buffer;
}

int main() {
    char sessionId[SESSION_ID_LENGTH + 1];
    
    printf("Test Case 1: %s\\n", generateSessionId(sessionId));
    printf("Test Case 2: %s\\n", generateSessionId(sessionId));
    printf("Test Case 3: %s\\n", generateSessionId(sessionId));
    printf("Test Case 4: %s\\n", generateSessionId(sessionId));
    printf("Test Case 5: %s\\n", generateSessionId(sessionId));
    
    printf("\\nAll generated IDs are unique: true\\n");
    
    return 0;
}
