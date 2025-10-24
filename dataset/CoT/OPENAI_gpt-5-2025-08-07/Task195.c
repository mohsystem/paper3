#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int *data;
    int size;
    int cap;
    int isMin; // 1 for min-heap, 0 for max-heap
} Heap;

static void heapInit(Heap *h, int initialCap, int isMin) {
    if (initialCap < 1) initialCap = 8;
    h->data = (int*)malloc((size_t)initialCap * sizeof(int));
    h->size = 0;
    h->cap = (h->data != NULL) ? initialCap : 0;
    h->isMin = isMin ? 1 : 0;
}

static void heapFree(Heap *h) {
    if (h && h->data) {
        free(h->data);
        h->data = NULL;
    }
    h->size = 0;
    h->cap = 0;
    h->isMin = 0;
}

static int heapCmp(Heap *h, int a, int b) {
    // return 1 if a should come before b
    return h->isMin ? (a < b) : (a > b);
}

static int heapEnsureCap(Heap *h, int need) {
    if (h->cap >= need) return 1;
    int newCap = h->cap ? h->cap : 8;
    while (newCap < need) {
        if (newCap > (1<<27)) { // prevent overflow
            newCap = need;
            break;
        }
        newCap <<= 1;
    }
    int *nd = (int*)realloc(h->data, (size_t)newCap * sizeof(int));
    if (!nd) return 0;
    h->data = nd;
    h->cap = newCap;
    return 1;
}

static int heapPush(Heap *h, int val) {
    if (!heapEnsureCap(h, h->size + 1)) return 0;
    int i = h->size++;
    h->data[i] = val;
    // sift up
    while (i > 0) {
        int p = (i - 1) / 2;
        if (heapCmp(h, h->data[i], h->data[p])) {
            int tmp = h->data[i]; h->data[i] = h->data[p]; h->data[p] = tmp;
            i = p;
        } else break;
    }
    return 1;
}

static int heapTop(Heap *h, int *ok) {
    if (h->size == 0) {
        if (ok) *ok = 0;
        return 0;
    }
    if (ok) *ok = 1;
    return h->data[0];
}

static int heapPop(Heap *h, int *ok) {
    if (h->size == 0) {
        if (ok) *ok = 0;
        return 0;
    }
    int ret = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    // sift down
    int i = 0;
    while (1) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        int best = i;
        if (l < h->size && heapCmp(h, h->data[l], h->data[best])) best = l;
        if (r < h->size && heapCmp(h, h->data[r], h->data[best])) best = r;
        if (best != i) {
            int tmp = h->data[i]; h->data[i] = h->data[best]; h->data[best] = tmp;
            i = best;
        } else break;
    }
    if (ok) *ok = 1;
    return ret;
}

typedef struct {
    Heap lower; // max-heap
    Heap upper; // min-heap
} MedianFinder;

static void MedianFinder_init(MedianFinder *mf) {
    heapInit(&mf->lower, 0, 0); // max-heap
    heapInit(&mf->upper, 0, 1); // min-heap
}

static void MedianFinder_free(MedianFinder *mf) {
    heapFree(&mf->lower);
    heapFree(&mf->upper);
}

static void MedianFinder_addNum(MedianFinder *mf, int num) {
    // push to lower
    heapPush(&mf->lower, num);
    // move largest from lower to upper
    int ok = 0;
    int top = heapPop(&mf->lower, &ok);
    if (ok) {
        heapPush(&mf->upper, top);
    }
    // ensure lower has equal or one more element than upper
    if (mf->upper.size > mf->lower.size) {
        int t2 = heapPop(&mf->upper, &ok);
        if (ok) heapPush(&mf->lower, t2);
    }
}

static double MedianFinder_findMedian(MedianFinder *mf) {
    if (mf->lower.size == 0) return 0.0; // defensive
    if (mf->lower.size > mf->upper.size) {
        int ok = 0;
        int lv = heapTop(&mf->lower, &ok);
        return ok ? (double)lv : 0.0;
    } else {
        int ok1 = 0, ok2 = 0;
        int lv = heapTop(&mf->lower, &ok1);
        int uv = heapTop(&mf->upper, &ok2);
        if (!ok1 || !ok2) return 0.0;
        return ((double)lv + (double)uv) / 2.0;
    }
}

static void run_test1(void) {
    MedianFinder mf; MedianFinder_init(&mf);
    MedianFinder_addNum(&mf, 1);
    MedianFinder_addNum(&mf, 2);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 1.5
    MedianFinder_addNum(&mf, 3);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 2.0
    MedianFinder_free(&mf);
}

static void run_test2(void) {
    MedianFinder mf; MedianFinder_init(&mf);
    MedianFinder_addNum(&mf, 2);
    MedianFinder_addNum(&mf, 3);
    MedianFinder_addNum(&mf, 4);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 3.0
    MedianFinder_free(&mf);
}

static void run_test3(void) {
    MedianFinder mf; MedianFinder_init(&mf);
    MedianFinder_addNum(&mf, 2);
    MedianFinder_addNum(&mf, 3);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 2.5
    MedianFinder_free(&mf);
}

static void run_test4(void) {
    MedianFinder mf; MedianFinder_init(&mf);
    MedianFinder_addNum(&mf, -100000);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // -100000.0
    MedianFinder_free(&mf);
}

static void run_test5(void) {
    MedianFinder mf; MedianFinder_init(&mf);
    MedianFinder_addNum(&mf, 5);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 5.0
    MedianFinder_addNum(&mf, 15);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 10.0
    MedianFinder_addNum(&mf, 1);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 5.0
    MedianFinder_addNum(&mf, 3);
    printf("%.6f\n", MedianFinder_findMedian(&mf)); // 4.0
    MedianFinder_free(&mf);
}

int main(void) {
    run_test1();
    run_test2();
    run_test3();
    run_test4();
    run_test5();
    return 0;
}