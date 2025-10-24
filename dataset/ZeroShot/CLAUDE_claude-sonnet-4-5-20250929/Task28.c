
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* whoLikesIt(char** names, int count) {
    char* result = (char*)malloc(200 * sizeof(char));
    
    if (count == 0) {
        strcpy(result, "no one likes this");
    } else if (count == 1) {
        sprintf(result, "%s likes this", names[0]);
    } else if (count == 2) {
        sprintf(result, "%s and %s like this", names[0], names[1]);
    } else if (count == 3) {
        sprintf(result, "%s, %s and %s like this", names[0], names[1], names[2]);
    } else {
        sprintf(result, "%s, %s and %d others like this", names[0], names[1], count - 2);
    }
    
    return result;
}

int main() {
    // Test case 1
    char** test1 = NULL;
    char* result1 = whoLikesIt(test1, 0);
    printf("%s\\n", result1);
    free(result1);
    
    // Test case 2
    char* test2[] = {"Peter"};
    char* result2 = whoLikesIt(test2, 1);
    printf("%s\\n", result2);
    free(result2);
    
    // Test case 3
    char* test3[] = {"Jacob", "Alex"};
    char* result3 = whoLikesIt(test3, 2);
    printf("%s\\n", result3);
    free(result3);
    
    // Test case 4
    char* test4[] = {"Max", "John", "Mark"};
    char* result4 = whoLikesIt(test4, 3);
    printf("%s\\n", result4);
    free(result4);
    
    // Test case 5
    char* test5[] = {"Alex", "Jacob", "Mark", "Max"};
    char* result5 = whoLikesIt(test5, 4);
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
