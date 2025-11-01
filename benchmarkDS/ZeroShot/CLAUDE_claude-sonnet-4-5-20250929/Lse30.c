
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

const char* secureRedirect(const char* target) {
    if (target == NULL || strlen(target) == 0) {
        return "/";
    }
    
    regex_t regex;
    int reti;
    
    // Compile regular expression
    reti = regcomp(&regex, "^[a-zA-Z0-9-]+\\\\.", REG_EXTENDED);
    if (reti) {
        return "/";
    }
    
    // Execute regular expression
    reti = regexec(&regex, target, 0, NULL, 0);
    regfree(&regex);
    
    if (!reti) {
        return target;
    } else {
        return "/";
    }
}

int main() {
    // Test cases
    printf("Test 1: %s\\n", secureRedirect("example.com"));  // Should return "example.com"
    printf("Test 2: %s\\n", secureRedirect("my-site.org"));  // Should return "my-site.org"
    printf("Test 3: %s\\n", secureRedirect("test123.net"));  // Should return "test123.net"
    printf("Test 4: %s\\n", secureRedirect(".invalid"));     // Should return "/"
    printf("Test 5: %s\\n", secureRedirect("no-period"));    // Should return "/"
    
    return 0;
}
