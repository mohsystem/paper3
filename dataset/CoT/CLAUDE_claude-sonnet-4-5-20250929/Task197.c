
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
    if (radius <= 0) {
        fprintf(stderr, "Radius must be positive\\n");
        return NULL;
    }
    
    Task197* obj = (Task197*)malloc(sizeof(Task197));
    if (obj == NULL) {
        return NULL;
    }
    
    obj->radius = radius;
    obj->x_center = x_center;
    obj->y_center = y_center;
    
    return obj;
}

void randPoint(Task197* obj, double* result) {
    if (obj == NULL || result == NULL) {
        return;
    }
    
    double angle = ((double)rand() / RAND_MAX) * 2 * M_PI;
    double r = sqrt((double)rand() / RAND_MAX) * obj->radius;
    
    result[0] = obj->x_center + r * cos(angle);
    result[1] = obj->y_center + r * sin(angle);
}

void freeTask197(Task197* obj) {
    if (obj != NULL) {
        free(obj);
    }
}

int main() {
    srand(time(NULL));
    
    Task197* solution1 = createTask197(1.0, 0.0, 0.0);
    if (solution1 != NULL) {
        double result[2];
        
        randPoint(solution1, result);
        printf("Test 1: [%f, %f]\\n", result[0], result[1]);
        
        randPoint(solution1, result);
        printf("Test 2: [%f, %f]\\n", result[0], result[1]);
        
        randPoint(solution1, result);
        printf("Test 3: [%f, %f]\\n", result[0], result[1]);
        
        freeTask197(solution1);
    }
    
    Task197* solution2 = createTask197(5.0, 2.0, 3.0);
    if (solution2 != NULL) {
        double result[2];
        
        randPoint(solution2, result);
        printf("Test 4: [%f, %f]\\n", result[0], result[1]);
        
        randPoint(solution2, result);
        printf("Test 5: [%f, %f]\\n", result[0], result[1]);
        
        freeTask197(solution2);
    }
    
    return 0;
}
