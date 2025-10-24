
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_IP_LENGTH 46
#define MAX_RESULT_LENGTH 256

// Safe string length check with maximum limit
// Returns length if valid, 0 if NULL or too long
size_t safe_strlen(const char* str, size_t max_len) {
    if (str == NULL) {
        return 0;
    }
    
    size_t len = 0;
    while (len < max_len && str[len] != '\\0') {
        len++;
    }
    
    // If we reached max_len without finding null terminator, invalid
    if (len == max_len && str[len] != '\\0') {
        return 0;
    }
    
    return len;
}

// Safe string copy with bounds checking
// Returns true on success, false on failure
bool safe_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return false;
    }
    
    size_t i = 0;
    while (i < dest_size - 1 && src[i] != '\\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\\0';
    
    // Check if source was fully copied
    return src[i] == '\\0';
}

// Validate IPv4 address
// Returns true if valid, false otherwise
bool validate_ipv4(const char* ip) {
    // Input validation
    if (ip == NULL) {
        return false;
    }
    
    size_t len = safe_strlen(ip, MAX_IP_LENGTH);
    if (len == 0 || len > 15) { // Max IPv4: "255.255.255.255" = 15 chars
        return false;
    }
    
    // Count dots and validate characters
    int dot_count = 0;
    for (size_t i = 0; i < len; i++) {
        if (ip[i] == '.') {
            dot_count++;
        } else if (!isdigit((unsigned char)ip[i])) {
            return false; // Invalid character
        }
    }
    
    // Must have exactly 3 dots
    if (dot_count != 3) {
        return false;
    }
    
    // Parse octets with buffer overflow protection
    char ip_copy[16]; // Enough for IPv4 + null terminator
    if (!safe_strcpy(ip_copy, sizeof(ip_copy), ip)) {
        return false;
    }
    
    char* token = NULL;
    char* saveptr = NULL;
    int octet_count = 0;
    
    token = strtok_r(ip_copy, ".", &saveptr);
    while (token != NULL) {
        octet_count++;
        
        // Check octet count
        if (octet_count > 4) {
            return false;
        }
        
        // Empty octet invalid
        if (token[0] == '\\0') {
            return false;
        }
        
        // Leading zeros not allowed (except "0" itself)
        if (strlen(token) > 1 && token[0] == '0') {
            return false;
        }
        
        // Length check to prevent overflow
        if (strlen(token) > 3) {
            return false;
        }
        
        // Safe string to integer conversion
        char* endptr = NULL;
        long value = strtol(token, &endptr, 10);
        
        // Check conversion success
        if (endptr == token || *endptr != '\\0') {
            return false;
        }
        
        // Range check: 0-255 for IPv4 octets
        if (value < 0 || value > 255) {
            return false;
        }
        
        token = strtok_r(NULL, ".", &saveptr);
    }
    
    // Must have exactly 4 octets
    return octet_count == 4;
}

// Validate IPv6 address
// Returns true if valid, false otherwise
bool validate_ipv6(const char* ip) {
    // Input validation
    if (ip == NULL) {
        return false;
    }
    
    size_t len = safe_strlen(ip, MAX_IP_LENGTH);
    if (len == 0 || len > 39) { // Max standard IPv6 length
        return false;
    }
    
    // Count colons and validate characters
    int colon_count = 0;
    bool has_double_colon = false;
    
    for (size_t i = 0; i < len; i++) {
        char c = ip[i];
        if (c == ':') {
            colon_count++;
            // Check for ::
            if (i > 0 && ip[i-1] == ':') {
                if (has_double_colon) {
                    return false; // Multiple :: not allowed
                }
                has_double_colon = true;
            }
            // Check for :::
            if (i > 1 && ip[i-1] == ':' && ip[i-2] == ':') {
                return false;
            }
        } else if (!isxdigit((unsigned char)c)) {
            return false; // Invalid character
        }
    }
    
    // Colon count validation
    if (colon_count < 2 || colon_count > 7) {
        return false;
    }
    
    // Simple validation: check groups
    char ip_copy[40]; // Enough for IPv6 + null
    if (!safe_strcpy(ip_copy, sizeof(ip_copy), ip)) {
        return false;
    }
    
    // Split and validate groups
    int group_count = 0;
    char* token = NULL;
    char* saveptr = NULL;
    
    token = strtok_r(ip_copy, ":", &saveptr);
    while (token != NULL && group_count < 8) {
        if (strlen(token) > 0) {
            group_count++;
            
            // Groups should be 1-4 hex digits
            size_t token_len = strlen(token);
            if (token_len > 4) {
                return false;
            }
            
            // Validate hex digits
            for (size_t i = 0; i < token_len; i++) {
                if (!isxdigit((unsigned char)token[i])) {
                    return false;
                }
            }
        }
        
        token = strtok_r(NULL, ":", &saveptr);
    }
    
    // With ::, groups should be < 8, without :: exactly 8
    if (has_double_colon) {
        return group_count < 8;
    } else {
        return group_count == 8;
    }
}

// Process and validate IP address
// Returns result string (caller must provide buffer with size)
bool process_ip_address(const char* ip, char* result, size_t result_size) {
    // Input validation
    if (ip == NULL || result == NULL || result_size == 0) {
        return false;
    }
    
    // Initialize result buffer
    memset(result, 0, result_size);
    
    // Check length
    size_t len = safe_strlen(ip, MAX_IP_LENGTH);
    if (len == 0) {
        snprintf(result, result_size, "Invalid: Empty IP address");
        return false;
    }
    
    // Create trimmed copy with bounds checking
    char trimmed[MAX_IP_LENGTH + 1];
    memset(trimmed, 0, sizeof(trimmed));
    
    // Trim leading whitespace
    size_t start = 0;
    while (start < len && isspace((unsigned char)ip[start])) {
        start++;
    }
    
    // Trim trailing whitespace
    size_t end = len;
    while (end > start && isspace((unsigned char)ip[end - 1])) {
        end--;
    }
    
    // Check if anything left after trimming
    if (end <= start) {
        snprintf(result, result_size, "Invalid: Empty IP address");
        return false;
    }
    
    // Copy trimmed portion with bounds check
    size_t copy_len = end - start;
    if (copy_len >= sizeof(trimmed)) {
        snprintf(result, result_size, "Invalid: IP address too long");
        return false;
    }
    
    memcpy(trimmed, ip + start, copy_len);
    trimmed[copy_len] = '\\0';
    
    // Detect IP version by presence of colon
    bool is_ipv6 = false;
    for (size_t i = 0; i < copy_len; i++) {
        if (trimmed[i] == ':') {
            is_ipv6 = true;
            break;
        }
    }
    
    // Validate and format result
    if (is_ipv6) {
        if (validate_ipv6(trimmed)) {
            snprintf(result, result_size, "Valid IPv6: %s", trimmed);
            return true;
        } else {
            snprintf(result, result_size, "Invalid IPv6: %s", trimmed);
            return false;
        }
    } else {
        if (validate_ipv4(trimmed)) {
            snprintf(result, result_size, "Valid IPv4: %s", trimmed);
            return true;
        } else {
            snprintf(result, result_size, "Invalid IPv4: %s", trimmed);
            return false;
        }
    }
}

int main(void) {
    // Test cases with various valid and invalid IP addresses
    const char* test_cases[] = {
        "192.168.1.1",           // Valid IPv4
        "256.1.1.1",             // Invalid IPv4 (out of range)
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",  // Valid IPv6
        "2001:db8::8a2e:370:7334",  // Valid IPv6 with compression
        "192.168.1.256"          // Invalid IPv4 (out of range)
    };
    
    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("IP Address Validation Results:\\n");
    printf("================================\\n");
    
    for (size_t i = 0; i < test_count; i++) {
        char result[MAX_RESULT_LENGTH];
        
        // Initialize result buffer to zero
        memset(result, 0, sizeof(result));
        
        if (process_ip_address(test_cases[i], result, sizeof(result))) {
            printf("%s\\n", result);
        } else {
            printf("%s\\n", result);
        }
    }
    
    return 0;
}
