
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_SESSIONS 1000
#define SESSION_ID_LENGTH 65
#define USER_ID_LENGTH 100
#define ATTRIBUTE_KEY_LENGTH 50
#define ATTRIBUTE_VALUE_LENGTH 256
#define MAX_ATTRIBUTES 10
#define SESSION_TIMEOUT_SECONDS 1800

typedef struct {
    char key[ATTRIBUTE_KEY_LENGTH];
    char value[ATTRIBUTE_VALUE_LENGTH];
} SessionAttribute;

typedef struct {
    char session_id[SESSION_ID_LENGTH];
    char user_id[USER_ID_LENGTH];
    time_t created_at;
    time_t last_accessed;
    SessionAttribute attributes[MAX_ATTRIBUTES];
    int attribute_count;
    int active;
} Session;

typedef struct {
    Session sessions[MAX_SESSIONS];
    int session_count;
} SessionManager;

static SessionManager global_manager = {0};

void bytes_to_hex(const unsigned char* bytes, size_t len, char* output) {
    for (size_t i = 0; i < len; i++) {
        sprintf(output + (i * 2), "%02x", bytes[i]);
    }
    output[len * 2] = '\\0';
}

void generate_session_id(char* session_id) {
    unsigned char random_bytes[32];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
        // Fallback to less secure random if OpenSSL fails
        srand((unsigned int)time(NULL));
        for (int i = 0; i < 32; i++) {
            random_bytes[i] = rand() % 256;
        }
    }
    
    SHA256(random_bytes, sizeof(random_bytes), hash);
    bytes_to_hex(hash, SHA256_DIGEST_LENGTH, session_id);
}

void clean_expired_sessions(SessionManager* manager) {
    time_t current_time = time(NULL);
    for (int i = 0; i < manager->session_count; i++) {
        if (manager->sessions[i].active) {
            if (difftime(current_time, manager->sessions[i].last_accessed) >= SESSION_TIMEOUT_SECONDS) {
                manager->sessions[i].active = 0;
                manager->sessions[i].attribute_count = 0;
                memset(manager->sessions[i].session_id, 0, SESSION_ID_LENGTH);
                memset(manager->sessions[i].user_id, 0, USER_ID_LENGTH);
            }
        }
    }
}

char* create_session(const char* user_id) {
    if (user_id == NULL || strlen(user_id) == 0) {
        return NULL;
    }
    
    SessionManager* manager = &global_manager;
    clean_expired_sessions(manager);
    
    // Find available slot
    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!manager->sessions[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        if (manager->session_count >= MAX_SESSIONS) {
            return NULL; // No available slots
        }
        slot = manager->session_count++;
    }
    
    Session* session = &manager->sessions[slot];
    generate_session_id(session->session_id);
    strncpy(session->user_id, user_id, USER_ID_LENGTH - 1);
    session->user_id[USER_ID_LENGTH - 1] = '\\0';
    session->created_at = time(NULL);
    session->last_accessed = time(NULL);
    session->attribute_count = 0;
    session->active = 1;
    
    return session->session_id;
}

int validate_session(const char* session_id) {
    if (session_id == NULL || strlen(session_id) == 0) {
        return 0;
    }
    
    SessionManager* manager = &global_manager;
    
    for (int i = 0; i < manager->session_count; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].session_id, session_id) == 0) {
            
            time_t current_time = time(NULL);
            if (difftime(current_time, manager->sessions[i].last_accessed) >= SESSION_TIMEOUT_SECONDS) {
                manager->sessions[i].active = 0;
                return 0;
            }
            
            manager->sessions[i].last_accessed = current_time;
            return 1;
        }
    }
    
    return 0;
}

char* get_session_user_id(const char* session_id) {
    if (!validate_session(session_id)) {
        return NULL;
    }
    
    SessionManager* manager = &global_manager;
    
    for (int i = 0; i < manager->session_count; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].session_id, session_id) == 0) {
            return manager->sessions[i].user_id;
        }
    }
    
    return NULL;
}

void terminate_session(const char* session_id) {
    if (session_id == NULL) {
        return;
    }
    
    SessionManager* manager = &global_manager;
    
    for (int i = 0; i < manager->session_count; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].session_id, session_id) == 0) {
            
            manager->sessions[i].active = 0;
            manager->sessions[i].attribute_count = 0;
            memset(manager->sessions[i].session_id, 0, SESSION_ID_LENGTH);
            memset(manager->sessions[i].user_id, 0, USER_ID_LENGTH);
            return;
        }
    }
}

int set_session_attribute(const char* session_id, const char* key, const char* value) {
    if (!validate_session(session_id) || key == NULL || value == NULL) {
        return 0;
    }
    
    SessionManager* manager = &global_manager;
    
    for (int i = 0; i < manager->session_count; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].session_id, session_id) == 0) {
            
            Session* session = &manager->sessions[i];
            
            // Check if attribute already exists
            for (int j = 0; j < session->attribute_count; j++) {
                if (strcmp(session->attributes[j].key, key) == 0) {
                    strncpy(session->attributes[j].value, value, ATTRIBUTE_VALUE_LENGTH - 1);
                    session->attributes[j].value[ATTRIBUTE_VALUE_LENGTH - 1] = '\\0';
                    return 1;
                }
            }
            
            // Add new attribute
            if (session->attribute_count < MAX_ATTRIBUTES) {
                strncpy(session->attributes[session->attribute_count].key, key, ATTRIBUTE_KEY_LENGTH - 1);
                session->attributes[session->attribute_count].key[ATTRIBUTE_KEY_LENGTH - 1] = '\\0';
                strncpy(session->attributes[session->attribute_count].value, value, ATTRIBUTE_VALUE_LENGTH - 1);
                session->attributes[session->attribute_count].value[ATTRIBUTE_VALUE_LENGTH - 1] = '\\0';
                session->attribute_count++;
                return 1;
            }
            
            return 0;
        }
    }
    
    return 0;
}

char* get_session_attribute(const char* session_id, const char* key) {
    if (!validate_session(session_id) || key == NULL) {
        return NULL;
    }
    
    SessionManager* manager = &global_manager;
    
    for (int i = 0; i < manager->session_count; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].session_id, session_id) == 0) {
            
            Session* session = &manager->sessions[i];
            
            for (int j = 0; j < session->attribute_count; j++) {
                if (strcmp(session->attributes[j].key, key) == 0) {
                    return session->attributes[j].value;
                }
            }
            
            return NULL;
        }
    }
    
    return NULL;
}

int main() {
    printf("=== Secure Session Management Test Cases ===\\n\\n");
    
    // Test Case 1: Create session for user
    printf("Test 1: Create session\\n");
    char* session_id1 = create_session("user123");
    if (session_id1) {
        printf("Session created: %s\\n", session_id1);
        printf("Valid: %s\\n\\n", validate_session(session_id1) ? "true" : "false");
    }
    
    // Test Case 2: Validate existing session
    printf("Test 2: Validate session\\n");
    int is_valid = validate_session(session_id1);
    printf("Session valid: %s\\n", is_valid ? "true" : "false");
    char* user_id = get_session_user_id(session_id1);
    printf("User ID: %s\\n\\n", user_id ? user_id : "null");
    
    // Test Case 3: Set and get session attributes
    printf("Test 3: Session attributes\\n");
    set_session_attribute(session_id1, "role", "admin");
    set_session_attribute(session_id1, "preference", "dark_mode");
    char* role = get_session_attribute(session_id1, "role");
    char* pref = get_session_attribute(session_id1, "preference");
    printf("Role: %s\\n", role ? role : "null");
    printf("Preference: %s\\n\\n", pref ? pref : "null");
    
    // Test Case 4: Multiple sessions
    printf("Test 4: Multiple sessions\\n");
    char* session_id2 = create_session("user456");
    char* session_id3 = create_session("user789");
    printf("Session 2 created: %s\\n", session_id2 ? session_id2 : "null");
    printf("Session 3 created: %s\\n", session_id3 ? session_id3 : "null");
    printf("Session 1 valid: %s\\n", validate_session(session_id1) ? "true" : "false");
    printf("Session 2 valid: %s\\n", validate_session(session_id2) ? "true" : "false");
    printf("Session 3 valid: %s\\n\\n", validate_session(session_id3) ? "true" : "false");
    
    // Test Case 5: Terminate session
    printf("Test 5: Terminate session\\n");
    terminate_session(session_id1);
    printf("Session 1 after termination: %s\\n", validate_session(session_id1) ? "true" : "false");
    printf("Session 2 still valid: %s\\n", validate_session(session_id2) ? "true" : "false");
    printf("Session 3 still valid: %s\\n", validate_session(session_id3) ? "true" : "false");
    
    // Cleanup
    terminate_session(session_id2);
    terminate_session(session_id3);
    
    return 0;
}
