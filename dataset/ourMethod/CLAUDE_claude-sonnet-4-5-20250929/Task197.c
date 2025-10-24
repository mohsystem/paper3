
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Structure to hold the circle parameters
typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

// Initialize the Solution object with circle parameters
// Security: Validate input parameters to ensure they meet constraints
Solution* solutionCreate(double radius, double x_center, double y_center) {
    // Input validation: radius must be positive and within constraints
    if (radius <= 0.0 || radius > 1e8) {
        return NULL;  // Fail closed on invalid input
    }
    
    // Input validation: center coordinates must be within constraints
    if (x_center < -1e7 || x_center > 1e7 || y_center < -1e7 || y_center > 1e7) {
        return NULL;  // Fail closed on invalid input
    }
    
    // Allocate memory for Solution structure
    Solution* obj = (Solution*)malloc(sizeof(Solution));
    if (obj == NULL) {
        return NULL;  // Check allocation result
    }
    
    // Initialize all fields explicitly
    obj->radius = radius;
    obj->x_center = x_center;
    obj->y_center = y_center;
    
    return obj;
}

// Generate a uniform random point inside the circle
// Security: Use proper random number generation and avoid undefined behavior
double* solutionRandPoint(Solution* obj, int* returnSize) {
    // Validate input pointer
    if (obj == NULL || returnSize == NULL) {
        if (returnSize != NULL) {
            *returnSize = 0;
        }
        return NULL;  // Fail closed on null input
    }
    
    // Allocate memory for result array [x, y]
    double* result = (double*)malloc(2 * sizeof(double));
    if (result == NULL) {
        *returnSize = 0;
        return NULL;  // Check allocation result
    }
    
    // Initialize result array to prevent uninitialized memory access
    result[0] = 0.0;
    result[1] = 0.0;
    
    // Generate uniform random point using rejection sampling
    // To get uniform distribution, we use sqrt(r) for radius
    // Security: Use rand() with proper seeding and range validation
    
    // Generate random angle in [0, 2*PI)
    // RAND_MAX check to prevent division by zero
    if (RAND_MAX == 0) {
        free(result);
        *returnSize = 0;
        return NULL;
    }
    
    double angle = ((double)rand() / (double)RAND_MAX) * 2.0 * M_PI;
    
    // Generate random radius with uniform distribution
    // Use sqrt to ensure uniform area distribution
    double r = sqrt((double)rand() / (double)RAND_MAX) * obj->radius;
    
    // Convert polar coordinates to Cartesian coordinates
    // Bounds are implicitly safe as r <= radius and angle is normalized
    result[0] = obj->x_center + r * cos(angle);
    result[1] = obj->y_center + r * sin(angle);
    
    *returnSize = 2;
    return result;
}

// Free the Solution object
// Security: Prevent double-free and memory leaks
void solutionFree(Solution* obj) {
    if (obj != NULL) {
        // No sensitive data to clear in this structure
        free(obj);
    }
}

// Test driver with 5 test cases
int main() {
    // Seed random number generator once at program start
    // Security: Initialize PRNG for predictable behavior in testing
    srand((unsigned int)time(NULL));
    
    printf("Test Case 1: Circle with radius 1.0, center (0.0, 0.0)\\n");
    Solution* sol1 = solutionCreate(1.0, 0.0, 0.0);
    if (sol1 != NULL) {
        for (int i = 0; i < 3; i++) {
            int size = 0;
            double* point = solutionRandPoint(sol1, &size);
            if (point != NULL && size == 2) {
                printf("  Point %d: [%.5f, %.5f]\\n", i + 1, point[0], point[1]);
                free(point);  // Free allocated memory exactly once
            }
        }
        solutionFree(sol1);
    }
    
    printf("\\nTest Case 2: Circle with radius 10.0, center (5.0, -7.5)\\n");
    Solution* sol2 = solutionCreate(10.0, 5.0, -7.5);
    if (sol2 != NULL) {
        for (int i = 0; i < 3; i++) {
            int size = 0;
            double* point = solutionRandPoint(sol2, &size);
            if (point != NULL && size == 2) {
                printf("  Point %d: [%.5f, %.5f]\\n", i + 1, point[0], point[1]);
                free(point);
            }
        }
        solutionFree(sol2);
    }
    
    printf("\\nTest Case 3: Circle with radius 0.01, center (0.0, 0.0)\\n");
    Solution* sol3 = solutionCreate(0.01, 0.0, 0.0);
    if (sol3 != NULL) {
        for (int i = 0; i < 3; i++) {
            int size = 0;
            double* point = solutionRandPoint(sol3, &size);
            if (point != NULL && size == 2) {
                printf("  Point %d: [%.7f, %.7f]\\n", i + 1, point[0], point[1]);
                free(point);
            }
        }
        solutionFree(sol3);
    }
    
    printf("\\nTest Case 4: Circle with radius 100.0, center (-50.0, 50.0)\\n");
    Solution* sol4 = solutionCreate(100.0, -50.0, 50.0);
    if (sol4 != NULL) {
        for (int i = 0; i < 3; i++) {
            int size = 0;
            double* point = solutionRandPoint(sol4, &size);
            if (point != NULL && size == 2) {
                printf("  Point %d: [%.5f, %.5f]\\n", i + 1, point[0], point[1]);
                free(point);
            }
        }
        solutionFree(sol4);
    }
    
    printf("\\nTest Case 5: Invalid input (negative radius)\\n");
    Solution* sol5 = solutionCreate(-1.0, 0.0, 0.0);
    if (sol5 == NULL) {
        printf("  Correctly rejected invalid input\\n");
    }
    
    return 0;
}
