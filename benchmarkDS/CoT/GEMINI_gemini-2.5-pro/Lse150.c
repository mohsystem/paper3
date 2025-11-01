#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A struct to hold message data.
typedef struct {
    char* username;
    char* message;
} Message;

// In a real application, this would be a database.
// Here, we use a dynamically allocated array of pointers to simulate one.
Message** messageDatabase = NULL;
size_t messageCount = 0;
size_t dbCapacity = 0;

/**
 * @brief Sanitizes a string to prevent XSS by replacing HTML special characters.
 * IMPORTANT: This function allocates new memory for the returned string.
 * The caller is responsible for freeing this memory.
 * @param input The raw user input string.
 * @return A new, dynamically allocated string that is safe for HTML. Returns NULL on failure.
 */
char* sanitize(const char* input) {
    if (!input) return NULL;

    size_t needed_len = 1; // Start with 1 for null terminator
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&': needed_len += 5; break; // &amp;
            case '<': needed_len += 4; break; // &lt;
            case '>': needed_len += 4; break; // &gt;
            case '"': needed_len += 6; break; // &quot;
            case '\'': needed_len += 6; break; // &#x27;
            default: needed_len++; break;
        }
    }

    char* output = (char*)malloc(needed_len);
    if (!output) {
        perror("Failed to allocate memory for sanitization");
        return NULL;
    }

    char* ptr = output;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&': ptr += sprintf(ptr, "&amp;"); break;
            case '<': ptr += sprintf(ptr, "&lt;"); break;
            case '>': ptr += sprintf(ptr, "&gt;"); break;
            case '"': ptr += sprintf(ptr, "&quot;"); break;
            case '\'': ptr += sprintf(ptr, "&#x27;"); break;
            default: *ptr++ = input[i]; break;
        }
    }
    *ptr = '\0';
    return output;
}

/**
 * @brief Simulates inserting a user message into the database.
 * This method is safe from SQL Injection. In a real DB, use prepared statements.
 */
void insert_user_message_in_db(const char* username, const char* message) {
    printf("[DB] Inserting message for user: %s\n", username);
    
    // Resize the database if it's full
    if (messageCount >= dbCapacity) {
        size_t newCapacity = (dbCapacity == 0) ? 4 : dbCapacity * 2;
        Message** newDb = (Message**)realloc(messageDatabase, newCapacity * sizeof(Message*));
        if (!newDb) {
            perror("Failed to expand database");
            return;
        }
        messageDatabase = newDb;
        dbCapacity = newCapacity;
    }

    // Allocate memory for the new message
    Message* newMessage = (Message*)malloc(sizeof(Message));
    if (!newMessage) {
        perror("Failed to allocate message struct");
        return;
    }
    // strdup allocates memory and copies the string
    newMessage->username = strdup(username);
    newMessage->message = strdup(message);
    if (!newMessage->username || !newMessage->message) {
        perror("Failed to duplicate strings for message");
        free(newMessage->username); // free what might have been allocated
        free(newMessage->message);
        free(newMessage);
        return;
    }
    
    messageDatabase[messageCount++] = newMessage;
}

// Helper to check if a string is null or only whitespace
int is_string_blank(const char* str) {
    if (str == NULL) return 1;
    while (*str) {
        if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
            return 0; // Not blank
        }
        str++;
    }
    return 1; // Is blank
}

/**
 * @brief Simulates handling a POST request from a user.
 */
void post(const char* username, const char* message) {
    printf("\n--- Handling POST Request ---\n");

    // Security: Validate input to prevent null or empty submissions.
    if (is_string_blank(username) || is_string_blank(message)) {
        printf("Error: Username and message cannot be empty.\n");
        printf("--- End POST Request ---\n");
        return;
    }

    insert_user_message_in_db(username, message);
    
    printf("Action: Message posted successfully.\n");
    printf("Action: Redirecting user to main page...\n");
    printf("--- End POST Request ---\n");
}

/**
 * @brief Simulates rendering the main page.
 */
void showMainPage() {
    printf("\n======================================\n");
    printf("         Welcome to the Main Page\n");
    printf("======================================\n");

    printf("\n--- Message Board ---\n");
    if (messageCount == 0) {
        printf("(No messages yet)\n");
    } else {
        for (size_t i = 0; i < messageCount; i++) {
            // Security: Sanitize all output to prevent stored XSS.
            char* safeUsername = sanitize(messageDatabase[i]->username);
            char* safeMessage = sanitize(messageDatabase[i]->message);
            
            if (safeUsername && safeMessage) {
                printf("%s: %s\n", safeUsername, safeMessage);
            }
            
            // IMPORTANT: Free the memory allocated by sanitize()
            free(safeUsername);
            free(safeMessage);
        }
    }
    printf("--- End of Messages ---\n");

    printf("\n--- Submit a New Message (Form) ---\n");
    printf("[ Username: ___________ ]\n");
    printf("[ Message:  ___________ ]\n");
    printf("[ [SUBMIT] ]\n");
    printf("======================================\n\n");
}

// Function to free all allocated memory to prevent leaks
void cleanup_database() {
    for (size_t i = 0; i < messageCount; i++) {
        free(messageDatabase[i]->username);
        free(messageDatabase[i]->message);
        free(messageDatabase[i]);
    }
    free(messageDatabase);
}


int main() {
    showMainPage();

    // --- Test Cases ---

    // Test Case 1
    printf(">>> TEST CASE 1: Standard Post\n");
    post("Alice", "Hello everyone!");
    showMainPage();

    // Test Case 2
    printf(">>> TEST CASE 2: Another User Post\n");
    post("Bob", "Hi Alice, how are you?");
    showMainPage();

    // Test Case 3 (XSS attempt)
    printf(">>> TEST CASE 3: XSS Attempt\n");
    post("Mallory", "<script>alert('pwned')</script>");
    showMainPage();

    // Test Case 4 (Empty message)
    printf(">>> TEST CASE 4: Empty Message\n");
    post("Charlie", "      ");
    showMainPage();

    // Test Case 5
    printf(">>> TEST CASE 5: Mixed Content\n");
    post("David", "I think 5 > 3 & 2 < 4. What do you think?");
    showMainPage();

    // Clean up all dynamically allocated memory before exiting
    cleanup_database();
    return 0;
}