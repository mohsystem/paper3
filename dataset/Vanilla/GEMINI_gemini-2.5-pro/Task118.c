#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

const char* validateIPv4(const char* ip) {
    // Make a copy as strtok modifies the string
    char ip_copy[100];
    strncpy(ip_copy, ip, 99);
    ip_copy[99] = '\0';
    
    // Check for trailing dot
    if (ip_copy[strlen(ip_copy) - 1] == '.') {
        return "Neither";
    }

    char* part = strtok(ip_copy, ".");
    int count = 0;
    
    while (part != NULL) {
        count++;
        int len = strlen(part);
        if (len == 0 || len > 3) return "Neither";
        if (len > 1 && part[0] == '0') return "Neither";

        for (int i = 0; i < len; i++) {
            if (!isdigit(part[i])) {
                return "Neither";
            }
        }
        
        int num = atoi(part);
        if (num < 0 || num > 255) return "Neither";
        
        part = strtok(NULL, ".");
    }
    
    return (count == 4) ? "IPv4" : "Neither";
}

const char* validateIPv6(const char* ip) {
    char ip_copy[100];
    strncpy(ip_copy, ip, 99);
    ip_copy[99] = '\0';

    // Check for trailing colon
    if (ip_copy[strlen(ip_copy) - 1] == ':') {
        return "Neither";
    }

    char* part = strtok(ip_copy, ":");
    int count = 0;

    while (part != NULL) {
        count++;
        int len = strlen(part);
        if (len == 0 || len > 4) return "Neither";
        
        for (int i = 0; i < len; i++) {
            if (!isxdigit(part[i])) {
                return "Neither";
            }
        }
        
        part = strtok(NULL, ":");
    }

    return (count == 8) ? "IPv6" : "Neither";
}

const char* validateIPAddress(const char* IP) {
    if (IP == NULL) return "Neither";
    
    if (strchr(IP, '.')) {
        return validateIPv4(IP);
    } else if (strchr(IP, ':')) {
        return validateIPv6(IP);
    }
    
    return "Neither";
}

int main() {
    const char* testCases[] = {
        "172.16.254.1",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "256.256.256.256",
        "192.168.01.1",
        "g:h:i:j:k:l:m:n"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("IP: %s -> %s\n", testCases[i], validateIPAddress(testCases[i]));
    }

    return 0;
}