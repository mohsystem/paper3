
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#define SESSION_ID_BYTES 32
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2 + 1)

// Generate a cryptographically secure random session ID
// Uses OpenSSL's RAND_bytes for CSPRNG (CWE-338 mitigation)\n// Returns a hex-encoded string of 32 random bytes (64 hex characters)\n// Caller is responsible for freeing the returned string\nchar* generateSessionID(void) {\n    unsigned char randomBytes[SESSION_ID_BYTES];\n    char* sessionID = NULL;\n    \n    // Allocate memory for hex string (2 chars per byte + null terminator)\n    sessionID = (char*)malloc(SESSION_ID_HEX_LEN);\n    if (sessionID == NULL) {\n        fprintf(stderr, "Memory allocation failed\\n");\n        return NULL;\n    }\n    \n    // Initialize memory to ensure proper null termination\n    memset(sessionID, 0, SESSION_ID_HEX_LEN);\n    \n    // Use OpenSSL's cryptographically secure random number generator
    // This prevents predictable session IDs (CWE-330, CWE-338)
    if (RAND_bytes(randomBytes, SESSION_ID_BYTES) != 1) {
        fprintf(stderr, "Failed to generate secure random bytes\\n");
        free(sessionID);
        return NULL;
    }
    
    // Convert to hex string for URL-safe representation
    // Use snprintf to prevent buffer overflow (CWE-120, CWE-787)
    for (size_t i = 0; i < SESSION_ID_BYTES; i++) {
        int written = snprintf(sessionID + (i * 2), 3, "%02x", randomBytes[i]);
        if (written < 0 || written >= 3) {
            fprintf(stderr, "Failed to format session ID\\n");
            // Clear sensitive data before freeing
            memset(randomBytes, 0, SESSION_ID_BYTES);
            free(sessionID);
            return NULL;
        }
    }
    
    // Clear sensitive random bytes from memory
    memset(randomBytes, 0, SESSION_ID_BYTES);
    
    return sessionID;
}

int main(void) {
    char* sessionIDs[5] = {NULL};
    int success = 1;
    
    printf("Generating 5 unique session IDs:\\n\\n");
    
    // Generate 5 session IDs
    for (int i = 0; i < 5; i++) {
        sessionIDs[i] = generateSessionID();
        if (sessionIDs[i] == NULL) {
            fprintf(stderr, "Failed to generate session ID %d\\n", i + 1);
            success = 0;
            break;
        }
        printf("Session ID %d: %s\\n", i + 1, sessionIDs[i]);
    }
    
    if (success) {
        // Verify uniqueness
        printf("\\nVerifying uniqueness:\\n");
        printf("ID1 != ID2: %s\\n", (strcmp(sessionIDs[0], sessionIDs[1]) != 0) ? "PASS" : "FAIL");
        printf("ID2 != ID3: %s\\n", (strcmp(sessionIDs[1], sessionIDs[2]) != 0) ? "PASS" : "FAIL");
        printf("ID3 != ID4: %s\\n", (strcmp(sessionIDs[2], sessionIDs[3]) != 0) ? "PASS" : "FAIL");
        printf("ID4 != ID5: %s\\n", (strcmp(sessionIDs[3], sessionIDs[4]) != 0) ? "PASS" : "FAIL");
    }
    
    // Free all allocated memory
    for (int i = 0; i < 5; i++) {
        if (sessionIDs[i] != NULL) {
            // Clear session ID before freeing (defense in depth)
            memset(sessionIDs[i], 0, SESSION_ID_HEX_LEN);
            free(sessionIDs[i]);
            sessionIDs[i] = NULL;
        }
    }
    
    return success ? 0 : 1;
}
