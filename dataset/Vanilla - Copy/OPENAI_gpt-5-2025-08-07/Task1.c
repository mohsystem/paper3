#include <stdio.h>

int number(int stops[][2], int n) {
    int passengers = 0;
    for (int i = 0; i < n; ++i) {
        passengers += stops[i][0] - stops[i][1];
    }
    return passengers;
}

int main() {
    int t1[][2] = {{10,0},{3,5},{5,8}};
    int t2[][2] = {{3,0},{9,1},{4,10},{12,2},{6,1},{7,10}};
    int t3[][2] = {{0,0}};
    int t4[][2] = {{5,0},{0,0},{0,0}};
    int t5[][2] = {{21,0},{0,20},{1,0}};
    printf("%d\n", number(t1, sizeof(t1)/sizeof(t1[0])));
    printf("%d\n", number(t2, sizeof(t2)/sizeof(t2[0])));
    printf("%d\n", number(t3, sizeof(t3)/sizeof(t3[0])));
    printf("%d\n", number(t4, sizeof(t4)/sizeof(t4[0])));
    printf("%d\n", number(t5, sizeof(t5)/sizeof(t5[0])));
    return 0;
}