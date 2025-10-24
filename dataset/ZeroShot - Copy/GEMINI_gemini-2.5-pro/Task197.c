#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Define M_PI if it's not available in math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

/**
 * @brief Creates and initializes a Solution object.
 * @param radius The radius of the circle.
 * @param x_center The x-coordinate of the center.
 * @param y_center The y-coordinate of the center.
 * @return A pointer to the newly created Solution object.
 */
Solution* solutionCreate(double radius, double x_center, double y_center) {
    Solution* obj = (Solution*)malloc(sizeof(Solution));
    if (obj == NULL) return NULL;
    obj->radius = radius;
    obj->x_center = x_center;
    obj->y_center = y_center;
    return obj;
}

/**
 * @brief Generates a uniform random point inside the circle.
 * @param obj A pointer to the Solution object.
 * @return A pointer to a dynamically allocated array [x, y]. The caller must free this memory.
 */
double* randPoint(Solution* obj) {
    if (obj == NULL) return NULL;
    
    double* result = (double*)malloc(2 * sizeof(double));
    if (result == NULL) return NULL;

    // rand() / (double)RAND_MAX generates a random number in [0.0, 1.0]
    double rand_val_u = (double)rand() / RAND_MAX;
    double rand_val_v = (double)rand() / RAND_MAX;

    // Use polar coordinates with inverse transform sampling for uniform distribution
    double r = obj->radius * sqrt(rand_val_u); // For uniform distribution of area
    double theta = rand_val_v * 2.0 * M_PI;

    result[0] = obj->x_center + r * cos(theta);
    result[1] = obj->y_center + r * sin(theta);
    
    return result;
}

/**
 * @brief Frees the memory allocated for the Solution object.
 * @param obj A pointer to the Solution object to be freed.
 */
void solutionFree(Solution* obj) {
    if (obj != NULL) {
        free(obj);
    }
}

void run_test_case(const char* name, double r, double xc, double yc) {
    printf("\n%s (radius=%.1f, center=(%.1f, %.1f)):\n", name, r, xc, yc);
    Solution* sol = solutionCreate(r, xc, yc);
    if (sol == NULL) {
        printf("Failed to create solution object.\n");
        return;
    }
    for (int i = 0; i < 5; ++i) {
        double* point = randPoint(sol);
        if (point != NULL) {
            printf("[%f, %f]\n", point[0], point[1]);
            free(point); // Free the memory for the returned point
        } else {
            printf("Failed to generate point.\n");
        }
    }
    solutionFree(sol); // Free the memory for the solution object
}

int main() {
    // Seed the random number generator once at the start of the program
    srand(time(NULL));

    run_test_case("Test Case 1", 1.0, 0.0, 0.0);
    run_test_case("Test Case 2", 10.0, 5.0, -7.5);
    run_test_case("Test Case 3", 0.1, 100.0, 100.0);
    run_test_case("Test Case 4", 1e8, -1e7, 1e7);
    run_test_case("Test Case 5", 1.0, 1.0, 1.0);

    return 0;
}