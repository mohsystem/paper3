#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Define M_PI if not already defined (e.g., by some C standards)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

/**
 * @brief Initializes the solution object with circle parameters.
 *
 * @param radius The radius of the circle. Must be > 0.
 * @param x_center The x-coordinate of the circle's center.
 * @param y_center The y-coordinate of the circle's center.
 * @return A pointer to the newly allocated Solution object, or NULL on failure.
 */
Solution* solutionCreate(double radius, double x_center, double y_center) {
    Solution* obj = (Solution*)malloc(sizeof(Solution));
    if (obj == NULL) {
        perror("Failed to allocate memory for Solution");
        return NULL;
    }
    obj->radius = radius;
    obj->x_center = x_center;
    obj->y_center = y_center;
    return obj;
}

/**
 * @brief Generates a uniform random point inside the circle.
 *
 * This function uses inverse transform sampling to ensure uniform distribution
 * across the area of the circle.
 * A random angle `theta` is chosen uniformly from [0, 2*PI).
 * A random radius `r` is chosen such that its distribution is proportional to
 * the area, which is achieved by `r = R * sqrt(u)` where `u` is uniform in [0, 1].
 *
 * @param obj A pointer to the initialized Solution object.
 * @return A pointer to a dynamically allocated array of two doubles [x, y]
 *         representing the point coordinates. The caller is responsible for
 *         freeing this memory. Returns NULL on failure.
 */
double* solutionRandPoint(Solution* obj) {
    if (obj == NULL) {
        return NULL;
    }

    // Generate two independent random numbers in [0.0, 1.0]
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;

    // Calculate the random radius with a square root to ensure uniform area distribution
    double r = obj->radius * sqrt(u1);
    
    // Calculate the random angle uniformly
    double theta = 2.0 * M_PI * u2;

    double* point = (double*)malloc(2 * sizeof(double));
    if (point == NULL) {
        perror("Failed to allocate memory for point");
        return NULL;
    }

    point[0] = obj->x_center + r * cos(theta);
    point[1] = obj->y_center + r * sin(theta);

    return point;
}

/**
 * @brief Frees the memory allocated for the Solution object.
 *
 * @param obj A pointer to the Solution object to be freed.
 */
void solutionFree(Solution* obj) {
    if (obj != NULL) {
        free(obj);
    }
}

int main() {
    // Seed the random number generator once at the start of the program.
    srand((unsigned int)time(NULL));

    // Test Case 1: Example from prompt
    printf("Test Case 1: radius=1.00, center=(0.00, 0.00)\n");
    Solution* sol1 = solutionCreate(1.0, 0.0, 0.0);
    if (sol1) {
        for (int i = 0; i < 5; ++i) {
            double* point = solutionRandPoint(sol1);
            if (point) {
                printf("  Random point %d: [%.5f, %.5f]\n", i + 1, point[0], point[1]);
                free(point);
            } else {
                fprintf(stderr, "  Failed to generate point.\n");
            }
        }
        solutionFree(sol1);
    } else {
        fprintf(stderr, "Failed to create Solution object for test case 1.\n");
    }
    printf("\n");

    // Test Case 2: Different center and larger radius
    printf("Test Case 2: radius=10.00, center=(5.00, -5.00)\n");
    Solution* sol2 = solutionCreate(10.0, 5.0, -5.0);
    if (sol2) {
        for (int i = 0; i < 5; ++i) {
            double* point = solutionRandPoint(sol2);
            if (point) {
                printf("  Random point %d: [%.5f, %.5f]\n", i + 1, point[0], point[1]);
                free(point);
            } else {
                fprintf(stderr, "  Failed to generate point.\n");
            }
        }
        solutionFree(sol2);
    } else {
        fprintf(stderr, "Failed to create Solution object for test case 2.\n");
    }
    printf("\n");

    // Test Case 3: Small radius, large center coordinates
    printf("Test Case 3: radius=0.10, center=(100.00, 100.00)\n");
    Solution* sol3 = solutionCreate(0.1, 100.0, 100.0);
    if (sol3) {
        for (int i = 0; i < 5; ++i) {
            double* point = solutionRandPoint(sol3);
            if (point) {
                printf("  Random point %d: [%.5f, %.5f]\n", i + 1, point[0], point[1]);
                free(point);
            } else {
                fprintf(stderr, "  Failed to generate point.\n");
            }
        }
        solutionFree(sol3);
    } else {
        fprintf(stderr, "Failed to create Solution object for test case 3.\n");
    }
    printf("\n");

    // Test Case 4: Large radius, negative center coordinate
    printf("Test Case 4: radius=1000.00, center=(-1000.00, 0.00)\n");
    Solution* sol4 = solutionCreate(1000.0, -1000.0, 0.0);
    if (sol4) {
        for (int i = 0; i < 5; ++i) {
            double* point = solutionRandPoint(sol4);
            if (point) {
                printf("  Random point %d: [%.5f, %.5f]\n", i + 1, point[0], point[1]);
                free(point);
            } else {
                fprintf(stderr, "  Failed to generate point.\n");
            }
        }
        solutionFree(sol4);
    } else {
        fprintf(stderr, "Failed to create Solution object for test case 4.\n");
    }
    printf("\n");

    // Test Case 5: Floating point center coordinates
    printf("Test Case 5: radius=5.00, center=(3.14, 2.71)\n");
    Solution* sol5 = solutionCreate(5.0, 3.14, 2.71);
    if (sol5) {
        for (int i = 0; i < 5; ++i) {
            double* point = solutionRandPoint(sol5);
            if (point) {
                printf("  Random point %d: [%.5f, %.5f]\n", i + 1, point[0], point[1]);
                free(point);
            } else {
                fprintf(stderr, "  Failed to generate point.\n");
            }
        }
        solutionFree(sol5);
    } else {
        fprintf(stderr, "Failed to create Solution object for test case 5.\n");
    }
    printf("\n");

    return 0;
}