#include <stdio.h>

int nb_year(int p0, double percent, int aug, int p) {
    int years = 0;
    while (p0 < p) {
        p0 += (int)(p0 * percent / 100.0) + aug;
        years++;
    }
    return years;
}

int main() {
    printf("%d\n", nb_year(1000, 2, 50, 1200));           // Expected: 3
    printf("%d\n", nb_year(1500, 5, 100, 5000));          // Expected: 15
    printf("%d\n", nb_year(1500000, 2.5, 10000, 2000000)); // Expected: 10
    printf("%d\n", nb_year(1500, 0, 100, 2000));          // Expected: 5
    printf("%d\n", nb_year(100, 5, 0, 101));              // Expected: 1
    return 0;
}