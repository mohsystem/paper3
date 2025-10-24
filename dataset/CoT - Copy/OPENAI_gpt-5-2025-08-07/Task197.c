#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <stdbool.h>

// Chain-of-Through:
// 1) Problem understanding: Uniformly sample inside circle: r = R*sqrt(U), theta in [0, 2π).
// 2) Security requirements: Validate inputs; avoid undefined behavior; check finite values.
// 3) Secure coding generation: Use standard RNG carefully; avoid division by zero; no unsafe casts.
// 4) Code review: Use sqrt(U) approach; include circumference; check input constraints.
// 5) Secure code output: Encapsulate in functions; return structs; minimal global state.

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    double radius;
    double x_center;
    double y_center;
} Solution;

// Helper to check finiteness portably
static bool is_finite_c(double v) {
    return isfinite(v);
}

static Solution Solution_init(double radius, double x_center, double y_center) {
    if (!(radius > 0.0) || !is_finite_c(radius)) {
        fprintf(stderr, "Invalid radius: must be positive and finite.\n");
        exit(1);
    }
    if (!is_finite_c(x_center) || !is_finite_c(y_center)) {
        fprintf(stderr, "Invalid center coordinates: must be finite.\n");
        exit(1);
    }
    Solution s;
    s.radius = radius;
    s.x_center = x_center;
    s.y_center = y_center;
    return s;
}

// Returns uniform double in [0,1]
static double uniform01(void) {
    // RAND_MAX may be small; combine two calls to improve granularity
    // but for simplicity and portability, a single call suffices.
    return (double)rand() / (double)RAND_MAX;
}

static Point Solution_randPoint(const Solution* s) {
    double u = uniform01();            // [0,1]
    double theta = uniform01() * (2.0 * M_PI); // [0,2π)
    double r = sqrt(u) * s->radius;
    Point p;
    p.x = s->x_center + r * cos(theta);
    p.y = s->y_center + r * sin(theta);
    return p;
}

int main(void) {
    // Seed RNG once
    srand((unsigned int)time(NULL));

    // 5 test cases
    Solution s1 = Solution_init(1.0, 0.0, 0.0);
    Solution s2 = Solution_init(3.5, 2.0, -1.0);
    Solution s3 = Solution_init(10.0, 1000.0, -2000.0);
    Solution s4 = Solution_init(0.0001, -0.5, 0.5);
    Solution s5 = Solution_init(5.0, -100.0, 100.0);

    Point p1 = Solution_randPoint(&s1);
    Point p2 = Solution_randPoint(&s2);
    Point p3 = Solution_randPoint(&s3);
    Point p4 = Solution_randPoint(&s4);
    Point p5 = Solution_randPoint(&s5);

    printf("[%.10f, %.10f]\n", p1.x, p1.y);
    printf("[%.10f, %.10f]\n", p2.x, p2.y);
    printf("[%.10f, %.10f]\n", p3.x, p3.y);
    printf("[%.10f, %.10f]\n", p4.x, p4.y);
    printf("[%.10f, %.10f]\n", p5.x, p5.y);

    return 0;
}