
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* whoLikesIt(char** names, int length) {
    char* result = (char*)malloc(200 * sizeof(char));
    
    if (result == NULL) {
        return NULL;
    }
    
    if (names == NULL || length == 0) {
        strcpy(result, "no one likes this");
    } else if (length == 1) {
        snprintf(result, 200, "%s likes this", names[0]);
    } else if (length == 2) {
        snprintf(result, 200, "%s and %s like this", names[0], names[1]);
    } else if (length == 3) {
        snprintf(result, 200, "%s, %s and %s like this", names[0], names[1], names[2]);
    } else {
        snprintf(result, 200, "%s, %s and %d others like this", names[0], names[1], length - 2);
    }
    
    return result;
}

int main() {
    char* result;
    
    // Test case 1
    result = whoLikesIt(NULL, 0);
    printf("%s\\n", result);
    free(result);
    
    // Test case 2
    char* names1[] = {"Peter"};
    result = whoLikesIt(names1, 1);
    printf("%s\\n", result);
    free(result);
    
    // Test case 3
    char* names2[] = {"Jacob", "Alex"};
    result = whoLikesIt(names2, 2);
    printf("%s\\n", result);
    free(result);
    
    // Test case 4
    char* names3[] = {"Max", "John", "Mark"};
    result = whoLikesIt(names3, 3);
    printf("%s\\n", result);
    free(result);
    
    // Test case 5
    char* names4[] = {"Alex", "Jacob", "Mark", "Max"};
    result = whoLikesIt(names4, 4);
    printf("%s\\n", result);
    free(result);
    
    return 0;
}
