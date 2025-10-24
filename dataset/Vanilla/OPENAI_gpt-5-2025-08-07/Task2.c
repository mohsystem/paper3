#include <stdio.h>
#include <stdlib.h>

char* createPhoneNumber(const int numbers[10]) {
    if (!numbers) return NULL;
    char* s = (char*)malloc(15);
    if (!s) return NULL;
    snprintf(s, 15, "(%d%d%d) %d%d%d-%d%d%d%d",
             numbers[0], numbers[1], numbers[2],
             numbers[3], numbers[4], numbers[5],
             numbers[6], numbers[7], numbers[8], numbers[9]);
    return s;
}

int main() {
    int t1[10] = {1,2,3,4,5,6,7,8,9,0};
    int t2[10] = {0,0,0,0,0,0,0,0,0,0};
    int t3[10] = {9,8,7,6,5,4,3,2,1,0};
    int t4[10] = {5,5,5,1,2,3,4,5,6,7};
    int t5[10] = {1,2,3,1,2,3,1,2,3,4};

    char* s1 = createPhoneNumber(t1);
    char* s2 = createPhoneNumber(t2);
    char* s3 = createPhoneNumber(t3);
    char* s4 = createPhoneNumber(t4);
    char* s5 = createPhoneNumber(t5);

    if (s1) { printf("%s\n", s1); free(s1); }
    if (s2) { printf("%s\n", s2); free(s2); }
    if (s3) { printf("%s\n", s3); free(s3); }
    if (s4) { printf("%s\n", s4); free(s4); }
    if (s5) { printf("%s\n", s5); free(s5); }

    return 0;
}