
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_IP_LENGTH 100

/**
 * Validates and processes IPv4 addresses
 * Prevents injection attacks and ensures strict validation
 */
bool validate_ipv4(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return false;
    }
    
    // Prevent excessively long inputs (DoS prevention)
    if (strlen(ip) > 15) {
        return false;
    }
    
    char ip_copy[16];
    strncpy(ip_copy, ip, sizeof(ip_copy) - 1);
    ip_copy[sizeof(ip_copy) - 1] = '\\0';
    
    int octets[4];
    int octet_count = 0;
    char* token = strtok(ip_copy, ".");
    
    while (token != NULL && octet_count < 4) {
        // Check if token contains only digits
        for (int i = 0; token[i] != '\\0'; i++) {
            if (!isdigit(token[i])) {
                return false;
            }
        }
        
        // Reject leading zeros (except for "0" itself)
        if (strlen(token) > 1 && token[0] == '0') {
            return false;
        }
        
        int value = atoi(token);
        if (value < 0 || value > 255) {
            return false;
        }
        
        octets[octet_count++] = value;
        token = strtok(NULL, ".");
    }
    
    return octet_count == 4;
}

/**
 * Validates IPv6 addresses (basic validation)
 * Prevents injection attacks and ensures strict validation
 */
bool validate_ipv6(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return false;
    }
    
    // Prevent excessively long inputs (DoS prevention)
    if (strlen(ip) > 39) {
        return false;
    }
    
    int colon_count = 0;
    int double_colon = 0;
    int hex_count = 0;
    
    for (int i = 0; ip[i] != '\\0'; i++) {
        if (ip[i] == ':') {
            if (i > 0 && ip[i-1] == ':') {
                double_colon++;
                if (double_colon > 1) {
                    return false;
                }
            }
            colon_count++;
            hex_count = 0;
        } else if (isxdigit(ip[i])) {
            hex_count++;
            if (hex_count > 4) {
                return false;
            }
        } else {
            return false;
        }
    }
    
    // Basic validation for IPv6 structure
    return colon_count >= 2 && colon_count <= 7;
}

/**
 * Determines IP version and validates accordingly
 * Returns: "Valid IPv4", "Valid IPv6", or "Invalid IP"
 */
const char* process_ip_address(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return "Invalid IP";
    }
    
    // Prevent buffer overflow and DoS attacks
    if (strlen(ip) > MAX_IP_LENGTH) {
        return "Invalid IP";
    }
    
    // Create a trimmed copy
    char trimmed[MAX_IP_LENGTH + 1];
    int j = 0;
    for (int i = 0; ip[i] != '\\0' && j < MAX_IP_LENGTH; i++) {
        if (!isspace(ip[i])) {
            trimmed[j++] = ip[i];
        }
    }
    trimmed[j] = '\\0';
    
    if (validate_ipv4(trimmed)) {
        return "Valid IPv4";
    } else if (validate_ipv6(trimmed)) {
        return "Valid IPv6";
    } else {
        return "Invalid IP";
    }
}

/**
 * Normalizes IPv4 address (removes leading zeros safely)
 */
char* normalize_ipv4(const char* ip) {
    static char normalized[16];
    
    if (!validate_ipv4(ip)) {
        return NULL;
    }
    
    char ip_copy[16];
    strncpy(ip_copy, ip, sizeof(ip_copy) - 1);
    ip_copy[sizeof(ip_copy) - 1] = '\\0';
    
    int octets[4];
    int octet_count = 0;
    char* token = strtok(ip_copy, ".");
    
    while (token != NULL && octet_count < 4) {
        octets[octet_count++] = atoi(token);
        token = strtok(NULL, ".");
    }
    
    snprintf(normalized, sizeof(normalized), "%d.%d.%d.%d", 
             octets[0], octets[1], octets[2], octets[3]);
    
    return normalized;
}

int main() {
    printf("=== IP Address Validation Tests ===\\n\\n");
    
    // Test Case 1: Valid IPv4
    const char* test1 = "192.168.1.1";
    printf("Test 1: %s\\n", test1);
    printf("Result: %s\\n", process_ip_address(test1));
    printf("IPv4 Valid: %s\\n", validate_ipv4(test1) ? "true" : "false");
    printf("Normalized: %s\\n\\n", normalize_ipv4(test1));
    
    // Test Case 2: Invalid IPv4 (out of range)
    const char* test2 = "256.168.1.1";
    printf("Test 2: %s\\n", test2);
    printf("Result: %s\\n", process_ip_address(test2));
    printf("IPv4 Valid: %s\\n\\n", validate_ipv4(test2) ? "true" : "false");
    
    // Test Case 3: Valid IPv6
    const char* test3 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    printf("Test 3: %s\\n", test3);
    printf("Result: %s\\n", process_ip_address(test3));
    printf("IPv6 Valid: %s\\n\\n", validate_ipv6(test3) ? "true" : "false");
    
    // Test Case 4: Invalid IP (malicious input attempt)
    const char* test4 = "192.168.1.1; DROP TABLE users;";
    printf("Test 4: %s\\n", test4);
    printf("Result: %s\\n", process_ip_address(test4));
    printf("IPv4 Valid: %s\\n\\n", validate_ipv4(test4) ? "true" : "false");
    
    // Test Case 5: Edge case (localhost)
    const char* test5 = "127.0.0.1";
    printf("Test 5: %s\\n", test5);
    printf("Result: %s\\n", process_ip_address(test5));
    printf("IPv4 Valid: %s\\n", validate_ipv4(test5) ? "true" : "false");
    printf("Normalized: %s\\n\\n", normalize_ipv4(test5));
    
    return 0;
}
