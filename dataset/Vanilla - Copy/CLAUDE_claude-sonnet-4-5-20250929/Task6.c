
#include <stdio.h>

double findUniq(double arr[], int size) {
    // Compare first three elements to determine which is unique
    if (arr[0] == arr[1]) {
        // arr[0] is the common element
        double common = arr[0];
        for (int i = 0; i < size; i++) {
            if (arr[i] != common) {
                return arr[i];
            }
        }
    } else if (arr[0] == arr[2]) {
        // arr[0] is the common element, arr[1] is unique
        return arr[1];
    } else {
        // arr[1] or arr[2] is the common element, arr[0] is unique
        return arr[0];
    }
    return arr[0]; // Should not reach here
}

int main() {
    // Test case 1
    double test1[] = {1, 1, 1, 2, 1, 1};
    printf("%f\\n", findUniq(test1, 6)); // => 2.0
    
    // Test case 2
    double test2[] = {0, 0, 0.55, 0, 0};
    printf("%f\\n", findUniq(test2, 5)); // => 0.55
    
    // Test case 3
    double test3[] = {3, 3, 3, 3, 5};
    printf("%f\\n", findUniq(test3, 5)); // => 5.0
    
    // Test case 4
    double test4[] = {10, 9, 9, 9, 9};
    printf("%f\\n", findUniq(test4, 5)); // => 10.0
    
    // Test case 5
    double test5[] = {7.5, 7.5, 8.5, 7.5};
    printf("%f\\n", findUniq(test5, 4)); // => 8.5
    
    return 0;
}
