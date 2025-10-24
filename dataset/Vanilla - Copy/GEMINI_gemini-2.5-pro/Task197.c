#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

Solution* solutionCreate(double radius, double x_center, double y_center) {
    Solution* obj = (Solution*)malloc(sizeof(Solution));
    if (!obj) return NULL;
    obj->radius = radius;
    obj->x_center = x_center;
    obj->y_center = y_center;
    return obj;
}

double* randPoint(Solution* obj, int* returnSize) {
    double rand_for_angle = (double)rand() / RAND_MAX;
    double rand_for_radius = (double)rand() / RAND_MAX;
    
    double angle = 2.0 * M_PI * rand_for_angle;
    double r = obj->radius * sqrt(rand_for_radius);
    
    double x = obj->x_center + r * cos(angle);
    double y = obj->y_center + r * sin(angle);
    
    double* result = (double*)malloc(2 * sizeof(double));
    if(!result) {
        *returnSize = 0;
        return NULL;
    }
    result[0] = x;
    result[1] = y;
    *returnSize = 2;
    return result;
}

void solutionFree(Solution* obj) {
    if (obj) {
        free(obj);
    }
}

void printArray(double* arr, int size) {
    if (!arr) return;
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%.5f", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main() {
    srand(time(NULL));
    int returnSize;
    double* point;

    Solution* s1 = solutionCreate(1.0, 0.0, 0.0);
    point = randPoint(s1, &returnSize); printArray(point, returnSize); free(point);
    point = randPoint(s1, &returnSize); printArray(point, returnSize); free(point);
    point = randPoint(s1, &returnSize); printArray(point, returnSize); free(point);
    solutionFree(s1);

    Solution* s2 = solutionCreate(10.0, 5.0, -7.5);
    point = randPoint(s2, &returnSize); printArray(point, returnSize); free(point);
    point = randPoint(s2, &returnSize); printArray(point, returnSize); free(point);
    solutionFree(s2);

    Solution* s3 = solutionCreate(0.01, -100.0, 100.0);
    point = randPoint(s3, &returnSize); printArray(point, returnSize); free(point);
    solutionFree(s3);
    
    Solution* s4 = solutionCreate(1000.0, 0.0, 0.0);
    point = randPoint(s4, &returnSize); printArray(point, returnSize); free(point);
    solutionFree(s4);

    Solution* s5 = solutionCreate(2.0, 2.0, 2.0);
    point = randPoint(s5, &returnSize); printArray(point, returnSize); free(point);
    solutionFree(s5);
    
    return 0;
}