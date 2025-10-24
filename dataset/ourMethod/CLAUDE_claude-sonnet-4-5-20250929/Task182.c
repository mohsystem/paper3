
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Security: Define maximum input size to prevent excessive memory allocation
#define MAX_INPUT_LENGTH 100
#define PHONE_NUMBER_LENGTH 10

// Clean phone number according to NANP rules
// Returns true on success, false on error
// Output buffer must be at least 11 bytes (10 digits + null terminator)
bool cleanPhoneNumber(const char* input, char* output, size_t output_size, char* error_msg, size_t error_size) {
    // Security: Validate all input pointers
    if (input == NULL || output == NULL || error_msg == NULL) {
        return false;
    }
    
    // Security: Ensure output buffer is large enough
    if (output_size < PHONE_NUMBER_LENGTH + 1) {
        snprintf(error_msg, error_size, "Output buffer too small");
        return false;
    }
    
    // Security: Validate input length to prevent excessive processing
    size_t input_len = strnlen(input, MAX_INPUT_LENGTH + 1);
    if (input_len > MAX_INPUT_LENGTH) {
        snprintf(error_msg, error_size, "Input too long");
        return false;
    }
    
    // Extract only digits from input
    char digits[12]; // 11 digits max + null terminator
    size_t digit_count = 0;
    
    for (size_t i = 0; i < input_len && input[i] != '\\0'; i++) {
        char c = input[i];
        
        // Security: Only accept ASCII digits
        if (c >= '0' && c <= '9') {
            // Security: Prevent buffer overflow
            if (digit_count >= 11) {
                snprintf(error_msg, error_size, "Too many digits");
                return false;
            }
            digits[digit_count++] = c;
        } else if (c == '+' || c == '-' || c == '(' || c == ')' || 
                   c == '.' || c == ' ') {
            // Allow common punctuation and whitespace
            continue;
        } else if (!isprint((unsigned char)c)) {
            // Security: Reject non-printable characters
            snprintf(error_msg, error_size, "Invalid character in input");
            return false;
        }
        // Other printable characters are silently ignored
    }
    
    // Security: Null-terminate the digits array
    digits[digit_count] = '\\0';
    
    // Process country code
    const char* phone_start = digits;
    size_t phone_len = digit_count;
    
    if (digit_count == 11) {
        // Security: Validate country code is exactly '1'
        if (digits[0] != '1') {
            snprintf(error_msg, error_size, "Invalid country code (must be 1)");
            return false;
        }
        phone_start = digits + 1;
        phone_len = 10;
    } else if (digit_count != 10) {
        snprintf(error_msg, error_size, "Invalid phone number length");
        return false;
    }
    
    // Validate NANP format: NXX NXX-XXXX
    // Area code must start with 2-9
    if (phone_start[0] < '2' || phone_start[0] > '9') {
        snprintf(error_msg, error_size, "Area code must start with digit 2-9");
        return false;
    }
    
    // Exchange code must start with 2-9
    if (phone_start[3] < '2' || phone_start[3] > '9') {
        snprintf(error_msg, error_size, "Exchange code must start with digit 2-9");
        return false;
    }
    
    // Security: Use bounded copy with explicit size check
    if (phone_len != PHONE_NUMBER_LENGTH) {
        snprintf(error_msg, error_size, "Internal error: unexpected length");
        return false;
    }
    
    // Copy result to output buffer with explicit bounds
    memcpy(output, phone_start, PHONE_NUMBER_LENGTH);
    output[PHONE_NUMBER_LENGTH] = '\\0';
    
    return true;
}

int main(void) {
    char output[PHONE_NUMBER_LENGTH + 1];
    char error_msg[256];
    
    // Test case 1: Country code with punctuation
    memset(output, 0, sizeof(output));
    memset(error_msg, 0, sizeof(error_msg));
    if (cleanPhoneNumber("+1 (613)-995-0253", output, sizeof(output), error_msg, sizeof(error_msg))) {
        printf("Test 1: %s\\n", output);
    } else {
        printf("Test 1 failed: %s\\n", error_msg);
    }
    
    // Test case 2: No country code with dashes
    memset(output, 0, sizeof(output));
    memset(error_msg, 0, sizeof(error_msg));
    if (cleanPhoneNumber("613-995-0253", output, sizeof(output), error_msg, sizeof(error_msg))) {
        printf("Test 2: %s\\n", output);
    } else {
        printf("Test 2 failed: %s\\n", error_msg);
    }
    
    // Test case 3: Country code with spaces
    memset(output, 0, sizeof(output));
    memset(error_msg, 0, sizeof(error_msg));
    if (cleanPhoneNumber("1 613 995 0253", output, sizeof(output), error_msg, sizeof(error_msg))) {
        printf("Test 3: %s\\n", output);
    } else {
        printf("Test 3 failed: %s\\n", error_msg);
    }
    
    // Test case 4: Dots as separators
    memset(output, 0, sizeof(output));
    memset(error_msg, 0, sizeof(error_msg));
    if (cleanPhoneNumber("613.995.0253", output, sizeof(output), error_msg, sizeof(error_msg))) {
        printf("Test 4: %s\\n", output);
    } else {
        printf("Test 4 failed: %s\\n", error_msg);
    }
    
    // Test case 5: Invalid - area code starts with 1
    memset(output, 0, sizeof(output));
    memset(error_msg, 0, sizeof(error_msg));
    if (cleanPhoneNumber("1 (123) 456-7890", output, sizeof(output), error_msg, sizeof(error_msg))) {
        printf("Test 5: %s\\n", output);
    } else {
        printf("Test 5 (expected to fail): %s\\n", error_msg);
    }
    
    return 0;
}
