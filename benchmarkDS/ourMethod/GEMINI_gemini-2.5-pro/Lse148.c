#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_MESSAGES 10
#define MAX_USERNAME_LEN 50
#define MAX_MESSAGE_LEN 280

// Note: In C, a struct with fixed-size arrays is a common and safer way
// to handle simple data structures, avoiding complex dynamic memory management.
typedef struct {
    // +1 for the null terminator
    char username[MAX_USERNAME_LEN + 1];
    char message[MAX_MESSAGE_LEN + 1];
} Message;

// Simulating a database with a global array and a counter
Message message_database[MAX_MESSAGES];
int message_count = 0;

/**
 * @brief Sanitizes input by escaping a few characters.
 * This is a simplified example. A real web application would need a comprehensive
 * library for security against XSS. This function focuses on safe buffer handling.
 * Rule #3: Ensure all buffer operations strictly adhere to boundaries.
 * @param input The raw input string.
 * @param output The buffer to store the sanitized string.
 * @param output_size The size of the output buffer.
 */
void sanitize(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) {
        if (output && output_size > 0) output[0] = '\0';
        return;
    }

    size_t i = 0, j = 0;
    while (input[i] != '\0' && j < output_size - 1) {
        char c = input[i];
        const char* replacement = NULL;
        size_t repl_len = 0;
        
        switch (c) {
            case '<': replacement = "&lt;"; repl_len = 4; break;
            case '>': replacement = "&gt;"; repl_len = 4; break;
            default: break;
        }

        if (replacement) {
            if (j + repl_len < output_size) {
                strncpy(output + j, replacement, repl_len);
                j += repl_len;
            } else {
                break; // Not enough space for replacement, truncate here.
            }
        } else {
            output[j] = c;
            j++;
        }
        i++;
    }
    output[j] = '\0'; // Ensure null termination
}

/**
 * @brief Checks if a string is NULL, empty, or contains only whitespace.
 * @param str The string to check.
 * @return True if the string is effectively blank, false otherwise.
 */
bool is_string_blank(const char* str) {
    if (!str) return true;
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}


/**
 * @brief Inserts a user message into our simulated database.
 * @param username The username of the poster.
 * @param message_text The message content.
 * @return true if insertion was successful, false otherwise.
 */
bool insert_user_message_in_db(const char* username, const char* message_text) {
    // Rule #1: Ensure all input is validated
    if (is_string_blank(username) || strlen(username) > MAX_USERNAME_LEN) {
        fprintf(stderr, "Error: Invalid username.\n");
        return false;
    }
    if (is_string_blank(message_text) || strlen(message_text) > MAX_MESSAGE_LEN) {
        fprintf(stderr, "Error: Invalid message.\n");
        return false;
    }
    if (message_count >= MAX_MESSAGES) {
        fprintf(stderr, "Error: Database is full.\n");
        return false;
    }
    
    Message* new_message = &message_database[message_count];
    
    // Sanitize and copy data using safe string functions (Rule #2 & #3)
    sanitize(username, new_message->username, sizeof(new_message->username));
    sanitize(message_text, new_message->message, sizeof(new_message->message));
    
    message_count++;
    printf("Message from '%s' stored successfully.\n", new_message->username);
    return true;
}

/**
 * @brief Called when a user submits a message.
 * @param username The username from the request.
 * @param message The message from the request.
 */
void post(const char* username, const char* message) {
    printf("\n--- Handling post request ---\n");
    // It calls the insert_user_message_in_db() function
    bool success = insert_user_message_in_db(username, message);

    if (success) {
        // It redirects the user to the main page (simulated)
        printf("Redirecting user to the main page...\n");
    } else {
        printf("Post failed. Not redirecting.\n");
    }
}

/**
 * @brief Displays all messages currently in the database.
 */
void display_messages() {
    printf("\n--- Displaying All Messages ---\n");
    if (message_count == 0) {
        printf("No messages to display.\n");
    } else {
        for (int i = 0; i < message_count; i++) {
            // Output is already sanitized from when it was inserted
            printf("%s: %s\n", message_database[i].username, message_database[i].message);
        }
    }
    printf("-----------------------------\n\n");
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1: Valid post
    post("Alice", "Hello, world!");
    display_messages();

    // Test Case 2: Another valid post
    post("Bob", "This is a test message.");
    display_messages();

    // Test Case 3: Invalid post (empty message)
    post("Charlie", " ");
    display_messages();

    // Test Case 4: Invalid post (username too long)
    char long_username[MAX_USERNAME_LEN + 2];
    memset(long_username, 'a', sizeof(long_username));
    long_username[sizeof(long_username) - 1] = '\0';
    post(long_username, "My username is too long.");
    display_messages();
    
    // Test Case 5: Post with characters that should be sanitized
    post("Eve", "<script>alert('XSS')</script>");
    display_messages();

    return 0;
}