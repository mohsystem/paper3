#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct to hold message data
typedef struct {
    char* username;
    char* message;
} Message;

// Global "database" implemented as a dynamic array
Message* database = NULL;
size_t db_size = 0;
size_t db_capacity = 0;

/**
 * Escapes HTML special characters in a string to prevent XSS.
 * The caller is responsible for freeing the returned string using free().
 */
char* escapeHtml(const char* input) {
    if (!input) return NULL;

    size_t input_len = strlen(input);
    // Over-allocate for worst case (e.g., all quotes -> &quot;) plus null terminator.
    char* escaped = (char*)malloc(input_len * 6 + 1);
    if (!escaped) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    char* p_out = escaped;
    const char* p_in = input;
    while (*p_in) {
        switch (*p_in) {
            case '&':  memcpy(p_out, "&amp;", 5);   p_out += 5; break;
            case '\"': memcpy(p_out, "&quot;", 6);  p_out += 6; break;
            case '\'': memcpy(p_out, "&#x27;", 6);  p_out += 6; break;
            case '<':  memcpy(p_out, "&lt;", 4);    p_out += 4; break;
            case '>':  memcpy(p_out, "&gt;", 4);    p_out += 4; break;
            default:   *p_out = *p_in;             p_out += 1; break;
        }
        p_in++;
    }
    *p_out = '\0';
    return escaped;
}

/**
 * Simulates inserting a user message into the database.
 * In a real application, this function must use a prepared statement to prevent SQL injection.
 */
void insert_user_message_in_db(const char* username, const char* message) {
    // Basic validation: do not insert null or empty strings.
    if (!username || !message || strlen(username) == 0 || strlen(message) == 0) {
        return;
    }

    // SQL INJECTION PREVENTION NOTE:
    // If using a real database, use a library (e.g., libpq for PostgreSQL) that supports
    // prepared statements to prevent SQL injection vulnerabilities.
    // Example with libpq:
    // const char *paramValues[2] = {username, message};
    // PQexecParams(conn, "INSERT INTO messages (username, message) VALUES ($1, $2)", ...);

    // Grow the dynamic array if it's full
    if (db_size >= db_capacity) {
        size_t new_capacity = (db_capacity == 0) ? 10 : db_capacity * 2;
        Message* new_db = (Message*)realloc(database, new_capacity * sizeof(Message));
        if (!new_db) {
            perror("Failed to reallocate database memory");
            return;
        }
        database = new_db;
        db_capacity = new_capacity;
    }

    // Use strdup to allocate memory and copy the strings.
    database[db_size].username = strdup(username);
    database[db_size].message = strdup(message);
    
    // Check if strdup succeeded
    if (!database[db_size].username || !database[db_size].message) {
        perror("Failed to duplicate strings for database entry");
        free(database[db_size].username); // free one if the other failed
        free(database[db_size].message);
        return; // Do not increment db_size
    }
    
    db_size++;
}

/**
 * Simulates fetching all messages and rendering them as an HTML string.
 * Calls escapeHtml on all user-provided data to prevent XSS.
 */
void display_messages() {
    printf("<h1>Messages</h1>\n<ul>\n");
    if (db_size == 0) {
        printf("<li>No messages yet.</li>\n");
    } else {
        for (size_t i = 0; i < db_size; ++i) {
            char* safeUsername = escapeHtml(database[i].username);
            char* safeMessage = escapeHtml(database[i].message);
            
            if (safeUsername && safeMessage) {
                printf("<li><b>%s:</b> %s</li>\n", safeUsername, safeMessage);
            }

            free(safeUsername);
            free(safeMessage);
        }
    }
    printf("</ul>");
}

/**
 * Simulates a request handler for the "/post" route.
 */
void routePost(const char* username, const char* message) {
    printf("--- Handling POST /post ---\n");
    printf("Received: username=%s, message=%s\n", username ? username : "(null)", message ? message : "(null)");
    insert_user_message_in_db(username, message);
    printf("-> Inserted data into DB.\n");
    printf("-> Redirecting to /\n");
    printf("-------------------------\n");
}

/**
 * Simulates a request handler for the "/" route.
 */
void routeGet() {
    printf("--- Handling GET / ---\n");
    printf("-> Displaying messages:\n");
    display_messages();
    printf("\n----------------------\n");
}

/**
 * Frees all dynamically allocated memory for the database.
 */
void cleanup_database() {
    for (size_t i = 0; i < db_size; ++i) {
        free(database[i].username);
        free(database[i].message);
    }
    free(database);
    database = NULL;
}

int main() {
    // --- 5 Test Cases ---
    printf(">>> Test Case 1: Post a regular message\n");
    routePost("Alice", "Hello World!");
    routeGet();

    printf("\n>>> Test Case 2: Post another message\n");
    routePost("Bob", "This is a test.");
    routeGet();

    printf("\n>>> Test Case 3: Post a message with HTML characters (XSS attempt)\n");
    routePost("Eve", "<script>alert('xss')</script>");
    routeGet(); // Output should show escaped HTML: &lt;script&gt;...

    printf("\n>>> Test Case 4: Post a message with other special characters\n");
    routePost("Charlie", "Me & You > Them");
    routeGet(); // Output should show escaped characters: Me &amp; You &gt; Them

    printf("\n>>> Test Case 5: Post empty/null data (should be handled gracefully)\n");
    routePost("", "An empty username");
    routePost("David", NULL);
    routePost(NULL, "A null username");
    printf("-> View after invalid posts (should be unchanged):\n");
    routeGet();

    // Clean up all dynamically allocated memory before exiting
    cleanup_database();
    
    return 0;
}