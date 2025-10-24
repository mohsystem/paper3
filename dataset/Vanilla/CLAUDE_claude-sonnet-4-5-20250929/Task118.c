
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

bool isValidIPv4(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return false;
    }
    
    char ipCopy[256];
    strcpy(ipCopy, ip);
    
    char* token;
    int count = 0;
    
    token = strtok(ipCopy, ".");
    while (token != NULL) {
        count++;
        if (count > 4) {
            return false;
        }
        
        int len = strlen(token);
        if (len == 0 || len > 3) {
            return false;
        }
        
        // Check for leading zeros
        if (len > 1 && token[0] == '0') {
            return false;
        }
        
        // Check if all characters are digits
        for (int i = 0; i < len; i++) {
            if (!isdigit(token[i])) {
                return false;
            }
        }
        
        int num = atoi(token);
        if (num < 0 || num > 255) {
            return false;
        }
        
        token = strtok(NULL, ".");
    }
    
    return count == 4;
}

bool isValidIPv6(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return false;
    }
    
    char ipCopy[256];
    strcpy(ipCopy, ip);
    
    // Check for double colon
    bool hasDoubleColon = strstr(ip, "::") != NULL;
    if (hasDoubleColon) {
        const char* first = strstr(ip, "::");
        const char* second = strstr(first + 2, "::");
        if (second != NULL) {
            return false;
        }
    }
    
    char* token;
    int count = 0;
    
    token = strtok(ipCopy, ":");
    while (token != NULL) {
        count++;
        if (count > 8) {
            return false;
        }
        
        int len = strlen(token);
        if (len == 0 && hasDoubleColon) {
            token = strtok(NULL, ":");
            continue;
        }
        if (len > 4) {
            return false;
        }
        
        for (int i = 0; i < len; i++) {
            if (!isxdigit(token[i])) {
                return false;
            }
        }
        
        token = strtok(NULL, ":");
    }
    
    return count >= 3;
}

const char* getIPVersion(const char* ip) {
    if (isValidIPv4(ip)) {
        return "IPv4";
    } else if (isValidIPv6(ip)) {
        return "IPv6";
    } else {
        return "Invalid";
    }
}

const char* normalizeIPv4(const char* ip) {
    if (!isValidIPv4(ip)) {
        return "Invalid IP";
    }
    return ip;
}

int main() {
    // Test case 1: Valid IPv4
    const char* test1 = "192.168.1.1";
    printf("Test 1: %s\\n", test1);
    printf("Valid IPv4: %s\\n", isValidIPv4(test1) ? "true" : "false");
    printf("IP Version: %s\\n", getIPVersion(test1));
    printf("\\n");
    
    // Test case 2: Invalid IPv4 (out of range)
    const char* test2 = "256.168.1.1";
    printf("Test 2: %s\\n", test2);
    printf("Valid IPv4: %s\\n", isValidIPv4(test2) ? "true" : "false");
    printf("IP Version: %s\\n", getIPVersion(test2));
    printf("\\n");
    
    // Test case 3: Valid IPv6
    const char* test3 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    printf("Test 3: %s\\n", test3);
    printf("Valid IPv6: %s\\n", isValidIPv6(test3) ? "true" : "false");
    printf("IP Version: %s\\n", getIPVersion(test3));
    printf("\\n");
    
    // Test case 4: IPv6 with double colon
    const char* test4 = "2001:db8::8a2e:370:7334";
    printf("Test 4: %s\\n", test4);
    printf("Valid IPv6: %s\\n", isValidIPv6(test4) ? "true" : "false");
    printf("IP Version: %s\\n", getIPVersion(test4));
    printf("\\n");
    
    // Test case 5: Invalid IP
    const char* test5 = "invalid.ip.address";
    printf("Test 5: %s\\n", test5);
    printf("Valid IPv4: %s\\n", isValidIPv4(test5) ? "true" : "false");
    printf("Valid IPv6: %s\\n", isValidIPv6(test5) ? "true" : "false");
    printf("IP Version: %s\\n", getIPVersion(test5));
    printf("\\n");
    
    return 0;
}
