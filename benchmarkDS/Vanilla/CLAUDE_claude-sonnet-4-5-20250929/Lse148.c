
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MESSAGES 100
#define MAX_LENGTH 256

typedef struct {
    char username[MAX_LENGTH];
    char message[MAX_LENGTH];
    time_t timestamp;
} Message;

// Global storage for messages
static Message messages[MAX_MESSAGES];
static int message_count = 0;

// Function to insert user message in database
void insert_user_message_in_db(const char* username, const char* message) {
    if (message_count < MAX_MESSAGES) {
        strncpy(messages[message_count].username, username, MAX_LENGTH - 1);
        messages[message_count].username[MAX_LENGTH - 1] = '\\0';
        
        strncpy(messages[message_count].message, message, MAX_LENGTH - 1);
        messages[message_count].message[MAX_LENGTH - 1] = '\\0';
        
        messages[message_count].timestamp = time(NULL);
        
        printf("Message inserted: %s - %s\\n", username, message);
        message_count++;
    } else {
        printf("Message storage full!\\n");
    }
}

// Post function to handle form submission
void post(const char* username, const char* message) {
    if (username != NULL && message != NULL && 
        strlen(username) > 0 && strlen(message) > 0) {
        insert_user_message_in_db(username, message);
    }
    // Redirect to main page (simulated)
    printf("Redirecting to main page...\\n");
}

// Get function to display messages
void get() {
    printf("=== Messages ===\\n");
    for (int i = 0; i < message_count; i++) {
        printf("%s: %s\\n", messages[i].username, messages[i].message);
    }
    printf("================\\n");
}

void display_messages() {
    get();
}

int main() {
    // Test cases
    printf("Test Case 1: Adding first message\\n");
    post("Alice", "Hello World!");
    
    printf("\\nTest Case 2: Adding second message\\n");
    post("Bob", "Hi there!");
    
    printf("\\nTest Case 3: Adding message with special characters\\n");
    post("Charlie", "Test @#$ message!");
    
    printf("\\nTest Case 4: Adding empty username\\n");
    post("", "Anonymous message");
    
    printf("\\nTest Case 5: Adding long message\\n");
    post("David", "This is a very long message to test the system's ability to handle lengthy text content.");
    
    printf("\\nDisplaying all messages:\\n");
    display_messages();
    
    return 0;
}
