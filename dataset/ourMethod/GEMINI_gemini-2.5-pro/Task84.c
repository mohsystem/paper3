#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For OpenSSL, ensure you have the development libraries installed.
// On Debian/Ubuntu: sudo apt-get install libssl-dev
// On RedHat/CentOS: sudo yum install openssl-devel
// Compile with: gcc this_file.c -o your_app -lssl -lcrypto
#include <openssl/rand.h>
#include <openssl/err.h>

#define SESSION_ID_BYTES 32
// Hex representation needs 2 chars per byte + 1 for null terminator.
#define SESSION_ID_STRING_LEN (SESSION_ID_BYTES * 2 + 1)

/**
 * Generates a cryptographically secure, random session ID using OpenSSL.
 * The ID is written as a hex-encoded string into the provided buffer.
 *
 * @param output_buffer A buffer to store the resulting hex string.
 * @param buffer_size The size of the output buffer. Must be at least SESSION_ID_STRING_LEN.
 * @return 0 on success, -1 on failure.
 */
int generate_session_id(char *output_buffer, size_t buffer_size) {
    if (output_buffer == NULL || buffer_size < SESSION_ID_STRING_LEN) {
        fprintf(stderr, "Error: Output buffer is NULL or too small.\n");
        return -1;
    }

    unsigned char random_bytes[SESSION_ID_BYTES];
    if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
        unsigned long err_code = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
        fprintf(stderr, "Failed to generate random bytes: %s\n", err_buf);
        return -1;
    }

    for (int i = 0; i < SESSION_ID_BYTES; i++) {
        // snprintf is used for safe string formatting to prevent buffer overflows.
        if (snprintf(output_buffer + (i * 2), 3, "%02x", random_bytes[i]) < 2) {
            fprintf(stderr, "Error during hex encoding.\n");
            return -1;
        }
    }
    output_buffer[SESSION_ID_STRING_LEN - 1] = '\0';

    return 0;
}

void run_test_cases() {
    printf("Generating 5 random and unique session IDs:\n");
    char session_id[SESSION_ID_STRING_LEN];

    for (int i = 0; i < 5; i++) {
        if (generate_session_id(session_id, sizeof(session_id)) == 0) {
            printf("Session ID %d: %s\n", i + 1, session_id);
            if (strlen(session_id) != SESSION_ID_STRING_LEN - 1) {
                fprintf(stderr, "Test case %d failed: Invalid session ID length.\n", i + 1);
            }
        } else {
            fprintf(stderr, "Failed to generate session ID for test case %d.\n", i + 1);
        }
    }
}

int main() {
    // Load the error strings for libcrypto for better error reporting.
    ERR_load_crypto_strings();

    run_test_cases();

    // Free the error strings.
    ERR_free_strings();
    return 0;
}