#include <stdio.h>
#include <ctype.h>

int str_to_int(const char* s) {
    if (!s) return 0;
    int i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    int sign = 1;
    if (s[i] == '+' || s[i] == '-') {
        if (s[i] == '-') sign = -1;
        i++;
    }
    long val = 0;
    while (s[i] && isdigit((unsigned char)s[i])) {
        val = val * 10 + (s[i] - '0');
        i++;
    }
    return (int)(sign * val);
}

void printAtIndex(const char* args[], int argc, int index) {
    if (index >= 0 && index < argc) {
        printf("%s\n", args[index]);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int idx = str_to_int(argv[1]);
        printAtIndex((const char**)argv, argc, idx);
        return 0;
    }

    // 5 test cases (run when no CLI index provided)
    const char* t1[] = {"Lse7", "2", "zero", "one", "two"};
    printAtIndex(t1, sizeof(t1)/sizeof(t1[0]), str_to_int(t1[1]));

    const char* t2[] = {"Lse7", "3", "red", "green", "blue", "yellow"};
    printAtIndex(t2, sizeof(t2)/sizeof(t2[0]), str_to_int(t2[1]));

    const char* t3[] = {"Lse7", "1", "hello"};
    printAtIndex(t3, sizeof(t3)/sizeof(t3[0]), str_to_int(t3[1]));

    const char* t4[] = {"Lse7", "4", "a", "b", "c", "d", "e"};
    printAtIndex(t4, sizeof(t4)/sizeof(t4[0]), str_to_int(t4[1]));

    const char* t5[] = {"Lse7", "0", "first"};
    printAtIndex(t5, sizeof(t5)/sizeof(t5[0]), str_to_int(t5[1]));

    return 0;
}