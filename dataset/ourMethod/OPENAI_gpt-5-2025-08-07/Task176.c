#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *pref;      // prefix products since last zero; pref[0] = 1
    int size;       // number of elements in pref (including sentinel)
    int capacity;   // allocated capacity
} ProductOfNumbers;

static void ProductOfNumbers_init(ProductOfNumbers *p) {
    if (!p) return;
    p->capacity = 16;
    p->pref = (int *)malloc((size_t)p->capacity * sizeof(int));
    if (!p->pref) {
        fprintf(stderr, "Allocation failed\n");
        exit(1);
    }
    p->size = 1;
    p->pref[0] = 1;
}

static void ProductOfNumbers_free(ProductOfNumbers *p) {
    if (!p) return;
    free(p->pref);
    p->pref = NULL;
    p->size = 0;
    p->capacity = 0;
}

static void ProductOfNumbers_reset(ProductOfNumbers *p) {
    if (!p) return;
    p->size = 1;
    p->pref[0] = 1;
}

static void ensure_capacity(ProductOfNumbers *p, int need) {
    if (need <= p->capacity) return;
    int newcap = p->capacity;
    while (newcap < need) {
        newcap *= 2;
        if (newcap <= 0) { // overflow guard
            newcap = need;
            break;
        }
    }
    int *np = (int *)realloc(p->pref, (size_t)newcap * sizeof(int));
    if (!np) {
        fprintf(stderr, "Reallocation failed\n");
        exit(1);
    }
    p->pref = np;
    p->capacity = newcap;
}

static void ProductOfNumbers_add(ProductOfNumbers *p, int num) {
    if (!p) return;
    if (num < 0 || num > 100) {
        fprintf(stderr, "num out of allowed range [0,100]\n");
        exit(1);
    }
    if (num == 0) {
        ProductOfNumbers_reset(p);
    } else {
        ensure_capacity(p, p->size + 1);
        long long next = (long long)p->pref[p->size - 1] * (long long)num; // fits 32-bit per constraints
        p->pref[p->size++] = (int)next;
    }
}

static int ProductOfNumbers_getProduct(ProductOfNumbers *p, int k) {
    if (!p) return 0;
    if (k <= 0) {
        fprintf(stderr, "k must be positive\n");
        exit(1);
    }
    if (k >= p->size) {
        return 0;
    }
    int num = p->pref[p->size - 1];
    int den = p->pref[p->size - 1 - k];
    return num / den;
}

static void header(const char *title) {
    printf("---- %s ----\n", title);
}

int main(void) {
    // Test Case 1: Example
    header("Test Case 1: Example");
    ProductOfNumbers p1;
    ProductOfNumbers_init(&p1);
    ProductOfNumbers_add(&p1, 3);
    ProductOfNumbers_add(&p1, 0);
    ProductOfNumbers_add(&p1, 2);
    ProductOfNumbers_add(&p1, 5);
    ProductOfNumbers_add(&p1, 4);
    printf("%d\n", ProductOfNumbers_getProduct(&p1, 2)); // 20
    printf("%d\n", ProductOfNumbers_getProduct(&p1, 3)); // 40
    printf("%d\n", ProductOfNumbers_getProduct(&p1, 4)); // 0
    ProductOfNumbers_add(&p1, 8);
    printf("%d\n", ProductOfNumbers_getProduct(&p1, 2)); // 32
    ProductOfNumbers_free(&p1);

    // Test Case 2: No zeros
    header("Test Case 2: No zeros");
    ProductOfNumbers p2;
    ProductOfNumbers_init(&p2);
    ProductOfNumbers_add(&p2, 1);
    ProductOfNumbers_add(&p2, 2);
    ProductOfNumbers_add(&p2, 3);
    ProductOfNumbers_add(&p2, 4);
    printf("%d\n", ProductOfNumbers_getProduct(&p2, 1)); // 4
    printf("%d\n", ProductOfNumbers_getProduct(&p2, 2)); // 12
    printf("%d\n", ProductOfNumbers_getProduct(&p2, 4)); // 24
    ProductOfNumbers_free(&p2);

    // Test Case 3: Multiple zeros and resets
    header("Test Case 3: Multiple zeros");
    ProductOfNumbers p3;
    ProductOfNumbers_init(&p3);
    ProductOfNumbers_add(&p3, 0);
    ProductOfNumbers_add(&p3, 9);
    ProductOfNumbers_add(&p3, 9);
    printf("%d\n", ProductOfNumbers_getProduct(&p3, 2)); // 81
    ProductOfNumbers_add(&p3, 0);
    printf("%d\n", ProductOfNumbers_getProduct(&p3, 1)); // 0
    ProductOfNumbers_add(&p3, 7);
    printf("%d\n", ProductOfNumbers_getProduct(&p3, 2)); // 0
    printf("%d\n", ProductOfNumbers_getProduct(&p3, 1)); // 7
    ProductOfNumbers_free(&p3);

    // Test Case 4: Crossing zero
    header("Test Case 4: Crossing zero");
    ProductOfNumbers p4;
    ProductOfNumbers_init(&p4);
    ProductOfNumbers_add(&p4, 2);
    ProductOfNumbers_add(&p4, 5);
    ProductOfNumbers_add(&p4, 0);
    ProductOfNumbers_add(&p4, 3);
    ProductOfNumbers_add(&p4, 4);
    printf("%d\n", ProductOfNumbers_getProduct(&p4, 2)); // 12
    printf("%d\n", ProductOfNumbers_getProduct(&p4, 3)); // 0
    printf("%d\n", ProductOfNumbers_getProduct(&p4, 5)); // 0
    ProductOfNumbers_free(&p4);

    // Test Case 5: Mixed segments
    header("Test Case 5: Mixed segments");
    ProductOfNumbers p5;
    ProductOfNumbers_init(&p5);
    ProductOfNumbers_add(&p5, 5);
    ProductOfNumbers_add(&p5, 2);
    ProductOfNumbers_add(&p5, 0);
    ProductOfNumbers_add(&p5, 1);
    ProductOfNumbers_add(&p5, 1);
    ProductOfNumbers_add(&p5, 1);
    ProductOfNumbers_add(&p5, 10);
    ProductOfNumbers_add(&p5, 0);
    ProductOfNumbers_add(&p5, 2);
    ProductOfNumbers_add(&p5, 2);
    ProductOfNumbers_add(&p5, 2);
    printf("%d\n", ProductOfNumbers_getProduct(&p5, 1)); // 2
    printf("%d\n", ProductOfNumbers_getProduct(&p5, 2)); // 4
    printf("%d\n", ProductOfNumbers_getProduct(&p5, 3)); // 8
    printf("%d\n", ProductOfNumbers_getProduct(&p5, 4)); // 0
    ProductOfNumbers_free(&p5);

    return 0;
}