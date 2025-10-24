#include <stdio.h>

long long find_nb(long long m) {
    long long totalVolume = 0;
    long long n = 0;
    while (totalVolume < m) {
        n++;
        totalVolume += n * n * n;
    }
    if (totalVolume == m) {
        return n;
    } else {
        return -1;
    }
}

int main() {
    printf("find_nb(1071225) -> %lld\n", find_nb(1071225));
    printf("find_nb(91716553919377LL) -> %lld\n", find_nb(91716553919377LL));
    printf("find_nb(4183059834009LL) -> %lld\n", find_nb(4183059834009LL));
    printf("find_nb(1) -> %lld\n", find_nb(1));
    printf("find_nb(135440716410000LL) -> %lld\n", find_nb(135440716410000LL));
    return 0;
}