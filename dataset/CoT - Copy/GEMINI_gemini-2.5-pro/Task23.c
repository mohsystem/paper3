#include <stdio.h>

long long find_nb(long long m) {
    long long total_volume = 0;
    long long n = 0;
    while (total_volume < m) {
        n++;
        long long cube_volume = n * n * n;
        total_volume += cube_volume;
    }

    if (total_volume == m) {
        return n;
    } else {
        return -1;
    }
}

int main() {
    printf("C Test Cases:\n");
    // Test Case 1: Example from description
    printf("find_nb(1071225) -> Expected: 45, Got: %lld\n", find_nb(1071225));
    // Test Case 2: Example from description, no solution
    printf("find_nb(91716553919377LL) -> Expected: -1, Got: %lld\n", find_nb(91716553919377LL));
    // Test Case 3: A large number with a solution
    printf("find_nb(4183059834009LL) -> Expected: 2022, Got: %lld\n", find_nb(4183059834009LL));
    // Test Case 4: Smallest possible input with a solution
    printf("find_nb(1) -> Expected: 1, Got: %lld\n", find_nb(1));
    // Test Case 5: Another large number with a solution
    printf("find_nb(135440716410000LL) -> Expected: 4824, Got: %lld\n", find_nb(135440716410000LL));
    return 0;
}