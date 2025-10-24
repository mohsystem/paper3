
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool validatePin(const char* pin) {
    if (pin == NULL) {
        return false;
    }
    
    int len = strlen(pin);
    if (len != 4 && len != 6) {
        return false;
    }
    
    for (int i = 0; i < len; i++) {
        if (!isdigit(pin[i])) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test cases
    printf("%d\\n", validatePin("1234"));   // 1 (true)
    printf("%d\\n", validatePin("12345"));  // 0 (false)
    printf("%d\\n", validatePin("a234"));   // 0 (false)
    printf("%d\\n", validatePin("123456")); // 1 (true)
    printf("%d\\n", validatePin(""));       // 0 (false)
    
    return 0;
}
