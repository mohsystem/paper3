
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isValidIPv4(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return false;
    }
    
    int num, dots = 0;
    char* ptr;
    char ipCopy[100];
    strncpy(ipCopy, ip, sizeof(ipCopy) - 1);
    ipCopy[sizeof(ipCopy) - 1] = '\\0';
    
    ptr = strtok(ipCopy, ".");
    
    if (ptr == NULL) {
        return false;
    }
    
    while (ptr) {
        if (!isdigit(*ptr)) {
            return false;
        }
        
        num = 0;
        for (int i = 0; ptr[i] != '\\0'; i++) {
            if (!isdigit(ptr[i])) {
                return false;
            }
            num = num * 10 + (ptr[i] - '0');
        }
        
        if (num < 0 || num > 255) {
            return false;
        }
        
        ptr = strtok(NULL, ".");
        if (ptr != NULL) {
            dots++;
        }
    }
    
    return dots == 3;
}

bool isValidIPv6(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return false;
    }
    
    int segmentCount = 0;
    int doubleColonFound = 0;
    int i = 0;
    int segmentLen = 0;
    
    while (ip[i] != '\\0') {
        if (ip[i] == ':') {
            if (i > 0 && ip[i-1] == ':') {
                doubleColonFound++;
                if (doubleColonFound > 1) {
                    return false;
                }
            }
            if (segmentLen > 0) {
                segmentCount++;
            }
            segmentLen = 0;
        } else if (isxdigit(ip[i])) {
            segmentLen++;
            if (segmentLen > 4) {
                return false;
            }
        } else {
            return false;
        }
        i++;
    }
    
    if (segmentLen > 0) {
        segmentCount++;
    }
    
    if (doubleColonFound) {
        return segmentCount <= 7;
    } else {
        return segmentCount == 8;
    }
}

const char* validateIP(const char* ip) {
    if (isValidIPv4(ip)) {
        return "Valid IPv4";
    } else if (isValidIPv6(ip)) {
        return "Valid IPv6";
    } else {
        return "Invalid IP";
    }
}

int main() {
    // Test cases
    const char* testCases[] = {
        "192.168.1.1",
        "256.1.1.1",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "::1",
        "invalid.ip.address"
    };
    
    int numTests = sizeof(testCases) / sizeof(testCases[0]);
    
    printf("IP Address Validation Results:\\n");
    printf("================================\\n");
    for (int i = 0; i < numTests; i++) {
        printf("IP: %s\\n", testCases[i]);
        printf("Result: %s\\n", validateIP(testCases[i]));
        printf("--------------------------------\\n");
    }
    
    return 0;
}
