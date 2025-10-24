#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int* data;
    int size;
    int cap;
} Heap;

// Secure allocation with overflow checks
static int safe_grow(Heap* h) {
    if (h->cap == 0) h->cap = 16;
    int newCap = h->cap * 2;
    if (newCap < h->cap) return 0; // overflow
    int* nd = (int*)realloc(h->data, (size_t)newCap * sizeof(int));
    if (!nd) return 0;
    h->data = nd;
    h->cap = newCap;
    return 1;
}

static void heap_init(Heap* h) {
    h->data = NULL;
    h->size = 0;
    h->cap = 0;
}

static void heap_free(Heap* h) {
    free(h->data);
    h->data = NULL;
    h->size = 0;
    h->cap = 0;
}

static void swap_int(int* a, int* b) {
    int t = *a; *a = *b; *b = t;
}

// Max-heap operations
static int maxheap_push(Heap* h, int val) {
    if (h->size >= h->cap) {
        if (!safe_grow(h)) return 0;
    }
    int i = h->size++;
    h->data[i] = val;
    // sift up
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[p] >= h->data[i]) break;
        swap_int(&h->data[p], &h->data[i]);
        i = p;
    }
    return 1;
}

static int maxheap_top(const Heap* h, int* out) {
    if (h->size <= 0) return 0;
    *out = h->data[0];
    return 1;
}

static int maxheap_pop(Heap* h, int* out) {
    if (h->size <= 0) return 0;
    *out = h->data[0];
    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        // sift down
        int i = 0;
        while (1) {
            int l = 2 * i + 1;
            int r = 2 * i + 2;
            int largest = i;
            if (l < h->size && h->data[l] > h->data[largest]) largest = l;
            if (r < h->size && h->data[r] > h->data[largest]) largest = r;
            if (largest == i) break;
            swap_int(&h->data[i], &h->data[largest]);
            i = largest;
        }
    }
    return 1;
}

// Min-heap operations
static int minheap_push(Heap* h, int val) {
    if (h->size >= h->cap) {
        if (!safe_grow(h)) return 0;
    }
    int i = h->size++;
    h->data[i] = val;
    // sift up
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[p] <= h->data[i]) break;
        swap_int(&h->data[p], &h->data[i]);
        i = p;
    }
    return 1;
}

static int minheap_top(const Heap* h, int* out) {
    if (h->size <= 0) return 0;
    *out = h->data[0];
    return 1;
}

static int minheap_pop(Heap* h, int* out) {
    if (h->size <= 0) return 0;
    *out = h->data[0];
    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        // sift down
        int i = 0;
        while (1) {
            int l = 2 * i + 1;
            int r = 2 * i + 2;
            int smallest = i;
            if (l < h->size && h->data[l] < h->data[smallest]) smallest = l;
            if (r < h->size && h->data[r] < h->data[smallest]) smallest = r;
            if (smallest == i) break;
            swap_int(&h->data[i], &h->data[smallest]);
            i = smallest;
        }
    }
    return 1;
}

typedef struct {
    Heap lower; // max-heap
    Heap upper; // min-heap
} MedianFinder;

MedianFinder* medianFinderCreate(void) {
    MedianFinder* mf = (MedianFinder*)calloc(1, sizeof(MedianFinder));
    if (!mf) return NULL;
    heap_init(&mf->lower);
    heap_init(&mf->upper);
    return mf;
}

void medianFinderFree(MedianFinder* mf) {
    if (!mf) return;
    heap_free(&mf->lower);
    heap_free(&mf->upper);
    free(mf);
}

int medianFinderAddNum(MedianFinder* mf, int num) {
    if (!mf) return 0;
    // push to lower (max-heap)
    if (!maxheap_push(&mf->lower, num)) return 0;

    // move top of lower to upper
    int moved;
    if (!maxheap_pop(&mf->lower, &moved)) return 0;
    if (!minheap_push(&mf->upper, moved)) return 0;

    // ensure lower has equal or one more element than upper
    if (mf->upper.size > mf->lower.size) {
        int back;
        if (!minheap_pop(&mf->upper, &back)) return 0;
        if (!maxheap_push(&mf->lower, back)) return 0;
    }
    return 1;
}

int medianFinderFindMedian(MedianFinder* mf, double* outMedian) {
    if (!mf || !outMedian) return 0;
    if (mf->lower.size == 0) return 0;
    if (mf->lower.size == mf->upper.size) {
        int a, b;
        if (!maxheap_top(&mf->lower, &a)) return 0;
        if (!minheap_top(&mf->upper, &b)) return 0;
        *outMedian = ((double)a + (double)b) / 2.0;
    } else {
        int a;
        if (!maxheap_top(&mf->lower, &a)) return 0;
        *outMedian = (double)a;
    }
    return 1;
}

static void run_case(const char* name, const int* ops, int opsLen, const int* vals, int valsLen, const double* expected, int expectedLen) {
    MedianFinder* mf = medianFinderCreate();
    if (!mf) {
        printf("%s: Failed to create MedianFinder\n", name);
        return;
    }
    int vi = 0;
    double results[1024];
    int ri = 0;
    for (int i = 0; i < opsLen; ++i) {
        if (ops[i] == 0) {
            if (vi >= valsLen) { printf("%s: Value index out of range\n", name); break; }
            if (!medianFinderAddNum(mf, vals[vi++])) {
                printf("%s: addNum failed\n", name);
                break;
            }
        } else if (ops[i] == 1) {
            double med = 0.0;
            if (!medianFinderFindMedian(mf, &med)) {
                printf("%s: findMedian failed\n", name);
                break;
            }
            results[ri++] = med;
        }
    }
    printf("%s results: [", name);
    for (int i = 0; i < ri; ++i) {
        if (i) printf(", ");
        printf("%.6f", results[i]);
    }
    printf("]\n");
    if (expected && expectedLen > 0) {
        printf("%s expected: [", name);
        for (int i = 0; i < expectedLen; ++i) {
            if (i) printf(", ");
            printf("%.6f", expected[i]);
        }
        printf("]\n");
    }
    printf("----\n");
    medianFinderFree(mf);
}

int main(void) {
    // Test 1: Example
    {
        int ops[] = {0,0,1,0,1};
        int vals[] = {1,2,3};
        double exp[] = {1.5, 2.0};
        run_case("Test1", ops, 5, vals, 3, exp, 2);
    }
    // Test 2: Single element
    {
        int ops[] = {0,1};
        int vals[] = {-100000};
        double exp[] = {-100000.0};
        run_case("Test2", ops, 2, vals, 1, exp, 1);
    }
    // Test 3: Even count increasing sequence
    {
        int ops[] = {0,0,0,0,1};
        int vals[] = {5,10,15,20};
        double exp[] = {12.5};
        run_case("Test3", ops, 5, vals, 4, exp, 1);
    }
    // Test 4: Duplicates
    {
        int ops[] = {0,0,0,0,0,1};
        int vals[] = {2,2,2,3,4};
        double exp[] = {2.0};
        run_case("Test4", ops, 6, vals, 5, exp, 1);
    }
    // Test 5: Mixed negatives and positives
    {
        int ops[] = {0,0,0,0,1};
        int vals[] = {-5,-1,-3,7};
        double exp[] = {-2.0};
        run_case("Test5", ops, 5, vals, 4, exp, 1);
    }
    return 0;
}