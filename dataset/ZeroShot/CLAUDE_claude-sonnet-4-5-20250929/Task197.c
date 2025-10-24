
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Task197;

Task197* createTask197(double radius, double x_center, double y_center) {
    Task197* obj = (Task197*)malloc(sizeof(Task197));
    obj->radius = radius;
    obj->x_center = x_center;
    obj->y_center = y_center;
    return obj;
}

double* randPoint(Task197* obj) {
    // Use square root to ensure uniform distribution
    double r = sqrt((double)rand() / RAND_MAX) * obj->radius;
    double theta = ((double)rand() / RAND_MAX) * 2 * M_PI;
    
    double* result = (double*)malloc(2 * sizeof(double));
    result[0] = obj->x_center + r * cos(theta);
    result[1] = obj->y_center + r * sin(theta);
    
    return result;
}

void freeTask197(Task197* obj) {
    free(obj);
}

int main() {
    srand(time(NULL));
    
    // Test case 1
    Task197* solution1 = createTask197(1.0, 0.0, 0.0);
    printf("Test case 1:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = randPoint(solution1);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    freeTask197(solution1);
    
    // Test case 2
    Task197* solution2 = createTask197(10.0, 5.0, -7.5);
    printf("\\nTest case 2:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = randPoint(solution2);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    freeTask197(solution2);
    
    // Test case 3
    Task197* solution3 = createTask197(0.01, 0.0, 0.0);
    printf("\\nTest case 3:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = randPoint(solution3);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    freeTask197(solution3);
    
    // Test case 4
    Task197* solution4 = createTask197(100.0, -50.0, 50.0);
    printf("\\nTest case 4:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = randPoint(solution4);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    freeTask197(solution4);
    
    // Test case 5
    Task197* solution5 = createTask197(5.5, 3.3, -2.2);
    printf("\\nTest case 5:\\n");
    for (int i = 0; i < 3; i++) {
        double* point = randPoint(solution5);
        printf("[%.5f, %.5f]\\n", point[0], point[1]);
        free(point);
    }
    freeTask197(solution5);
    
    return 0;
}
