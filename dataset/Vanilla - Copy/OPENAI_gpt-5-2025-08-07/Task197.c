#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

typedef struct {
    double x;
    double y;
} Point;

Solution Solution_init(double radius, double x_center, double y_center) {
    Solution s;
    s.radius = radius;
    s.x_center = x_center;
    s.y_center = y_center;
    return s;
}

static double urand() {
    return (rand() + 0.5) / (RAND_MAX + 1.0);
}

Point randPoint(Solution* s) {
    const double PI = acos(-1.0);
    double u = urand();
    double r = sqrt(u) * s->radius;
    double theta = 2.0 * PI * urand();
    Point p;
    p.x = s->x_center + r * cos(theta);
    p.y = s->y_center + r * sin(theta);
    return p;
}

int main() {
    srand((unsigned int)time(NULL));

    Solution tests[5];
    tests[0] = Solution_init(1.0, 0.0, 0.0);
    tests[1] = Solution_init(2.0, 1.0, 1.0);
    tests[2] = Solution_init(0.5, -2.0, -3.0);
    tests[3] = Solution_init(10.0, 5.0, -7.0);
    tests[4] = Solution_init(3.0, 100.0, 200.0);

    for (int i = 0; i < 5; ++i) {
        Point p = randPoint(&tests[i]);
        printf("[%.6f, %.6f]\n", p.x, p.y);
    }

    return 0;
}