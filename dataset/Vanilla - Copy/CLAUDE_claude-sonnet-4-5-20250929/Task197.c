
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

Solution* solutionCreate(double radius, double x_center, double y_center) {
    Solution* obj = (Solution*)malloc(sizeof(Solution));
    obj->radius = radius;
    obj->x_center = x_center;
    obj->y_center = y_center;
    srand(time(0));
    return obj;
}

double randomDouble() {
    return (double)rand() / RAND_MAX;
}

double* solutionRandPoint(Solution* obj) {
    // Use square root to ensure uniform distribution
    double r = sqrt(randomDouble()) * obj->radius;
    double theta = randomDouble() * 2 * M_PI;
    
    double* point = (double*)malloc(2 * sizeof(double));
    point[0] = obj->x_center + r * cos(theta);
    point[1] = obj->y_center + r * sin(theta);
    
    return point;
}

void solutionFree(Solution* obj) {
    free(obj);
}

int main() {
    // Test case 1
    Solution* solution1 = solutionCreate(1.0, 0.0, 0.0);
    printf("Test 1:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = solutionRandPoint(solution1);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    solutionFree(solution1);
    
    // Test case 2
    Solution* solution2 = solutionCreate(10.0, 5.0, -7.5);
    printf("\\nTest 2:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = solutionRandPoint(solution2);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    solutionFree(solution2);
    
    // Test case 3
    Solution* solution3 = solutionCreate(0.01, 0.0, 0.0);
    printf("\\nTest 3:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = solutionRandPoint(solution3);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    solutionFree(solution3);
    
    // Test case 4
    Solution* solution4 = solutionCreate(100.0, -50.0, 50.0);
    printf("\\nTest 4:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = solutionRandPoint(solution4);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    solutionFree(solution4);
    
    // Test case 5
    Solution* solution5 = solutionCreate(5.0, 10.0, 10.0);
    printf("\\nTest 5:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = solutionRandPoint(solution5);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    solutionFree(solution5);
    
    return 0;
}
