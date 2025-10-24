
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void getLastName(const char* name, char* lastName) {
    const char* lastSpace = strrchr(name, ' ');
    if (lastSpace != NULL) {
        strcpy(lastName, lastSpace + 1);
    } else {
        strcpy(lastName, name);
    }
}

int compareNames(const void* a, const void* b) {
    const char* name1 = *(const char**)a;
    const char* name2 = *(const char**)b;
    
    char lastName1[100], lastName2[100];
    getLastName(name1, lastName1);
    getLastName(name2, lastName2);
    
    int len1 = strlen(lastName1);
    int len2 = strlen(lastName2);
    
    if (len1 != len2) {
        return len1 - len2;
    }
    return strcmp(lastName1, lastName2);
}

void lastNameLensort(char** names, int count) {
    qsort(names, count, sizeof(char*), compareNames);
}

int main() {
    // Test case 1
    char* test1[] = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    int count1 = 5;
    lastNameLensort(test1, count1);
    printf("Test 1: ");
    for (int i = 0; i < count1; i++) {
        printf("%s, ", test1[i]);
    }
    printf("\\n");
    
    // Test case 2
    char* test2[] = {
        "John Doe",
        "Jane Smith",
        "Bob Lee"
    };
    int count2 = 3;
    lastNameLensort(test2, count2);
    printf("Test 2: ");
    for (int i = 0; i < count2; i++) {
        printf("%s, ", test2[i]);
    }
    printf("\\n");
    
    // Test case 3
    char* test3[] = {
        "Alice Johnson",
        "Charlie Brown",
        "David Green"
    };
    int count3 = 3;
    lastNameLensort(test3, count3);
    printf("Test 3: ");
    for (int i = 0; i < count3; i++) {
        printf("%s, ", test3[i]);
    }
    printf("\\n");
    
    // Test case 4
    char* test4[] = {
        "Emily White",
        "Frank Black",
        "Grace Blue"
    };
    int count4 = 3;
    lastNameLensort(test4, count4);
    printf("Test 4: ");
    for (int i = 0; i < count4; i++) {
        printf("%s, ", test4[i]);
    }
    printf("\\n");
    
    // Test case 5
    char* test5[] = {
        "Michael Anderson",
        "Sarah Williams",
        "Tom Martinez"
    };
    int count5 = 3;
    lastNameLensort(test5, count5);
    printf("Test 5: ");
    for (int i = 0; i < count5; i++) {
        printf("%s, ", test5[i]);
    }
    printf("\\n");
    
    return 0;
}
