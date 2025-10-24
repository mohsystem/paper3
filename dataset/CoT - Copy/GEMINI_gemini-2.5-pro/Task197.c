#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

const double PI = 3.14159265358979323846;

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

// Note: This function returns a dynamically allocated array of 2 doubles.
// The caller is responsible for freeing this memory using free().
double* randPoint(Solution* obj) {
    // Get two random numbers in [0.0, 1.0]
    double rand_val1 = (double)rand() / RAND_MAX;
    double rand_val2 = (double)rand() / RAND_MAX;

    // Generate a random angle and a random length, then convert to cartesian
    // The square root is necessary for uniform distribution over the area
    double angle = rand_val1 * 2 * PI;
    double length = sqrt(rand_val2) * obj->radius;

    double* point = (double*)malloc(2 * sizeof(double));
    if (point == NULL) {
        // Handle memory allocation failure
        return NULL;
    }
    
    point[0] = obj->x_center + length * cos(angle);
    point[1] = obj->y_center + length * sin(angle);
    
    return point;
}

int main() {
    // Seed the random number generator once at the start of the program
    srand(time(NULL));

    printf("C Output:\n");

    // Test Case 1: Unit circle at origin
    printf("\nTest Case 1: radius=1.0, center=(0.0, 0.0)\n");
    Solution sol1 = {1.0, 0.0, 0.0};
    for (int i = 0; i < 5; ++i) {
        double* p = randPoint(&sol1);
        if (p) {
            printf("[%f, %f]\n", p[0], p[1]);
            free(p);
        }
    }

    // Test Case 2: Larger circle with offset center
    printf("\nTest Case 2: radius=10.0, center=(5.0, -5.0)\n");
    Solution sol2 = {10.0, 5.0, -5.0};
    for (int i = 0; i < 5; ++i) {
        double* p = randPoint(&sol2);
        if (p) {
            printf("[%f, %f]\n", p[0], p[1]);
            free(p);
        }
    }

    // Test Case 3: Small radius
    printf("\nTest Case 3: radius=0.1, center=(1.0, 1.0)\n");
    Solution sol3 = {0.1, 1.0, 1.0};
    for (int i = 0; i < 5; ++i) {
        double* p = randPoint(&sol3);
        if (p) {
            printf("[%f, %f]\n", p[0], p[1]);
            free(p);
        }
    }
    
    // Test Case 4: Large radius and coordinates
    printf("\nTest Case 4: radius=1000.0, center=(-100.0, 200.0)\n");
    Solution sol4 = {1000.0, -100.0, 200.0};
    for (int i = 0; i < 5; ++i) {
        double* p = randPoint(&sol4);
        if (p) {
            printf("[%f, %f]\n", p[0], p[1]);
            free(p);
        }
    }
    
    // Test Case 5: Zero center, different radius
    printf("\nTest Case 5: radius=5.0, center=(0.0, 0.0)\n");
    Solution sol5 = {5.0, 0.0, 0.0};
    for (int i = 0; i < 5; ++i) {
        double* p = randPoint(&sol5);
        if (p) {
            printf("[%f, %f]\n", p[0], p[1]);
            free(p);
        }
    }

    return 0;
}