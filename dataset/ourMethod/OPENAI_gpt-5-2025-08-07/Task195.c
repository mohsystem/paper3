#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

typedef struct {
    int *low;      // max-heap (lower half)
    int *high;     // min-heap (upper half)
    int sizeLow;
    int sizeHigh;
    int capLow;
    int capHigh;
} MedianFinder;

static bool ensureCapacity(int **arr, int *cap, int needed) {
    if (needed <= *cap) return true;
    int newCap = *cap > 0 ? *cap : 16;
    while (newCap < needed) {
        if (newCap > 1000000 / 2) { // prevent overflow
            newCap = needed;
            break;
        }
        newCap *= 2;
    }
    int *newArr = (int *)realloc(*arr, (size_t)newCap * sizeof(int));
    if (!newArr) return false;
    *arr = newArr;
    *cap = newCap;
    return true;
}

static void swapInt(int *a, int *b) {
    int t = *a; *a = *b; *b = t;
}

/* Max-heap operations (lower half) */
static bool maxHeapPush(MedianFinder *mf, int val) {
    if (!ensureCapacity(&mf->low, &mf->capLow, mf->sizeLow + 1)) return false;
    int i = mf->sizeLow++;
    mf->low[i] = val;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (mf->low[p] >= mf->low[i]) break;
        swapInt(&mf->low[p], &mf->low[i]);
        i = p;
    }
    return true;
}

static int maxHeapTop(const MedianFinder *mf) {
    if (mf->sizeLow == 0) return 0;
    return mf->low[0];
}

static int maxHeapPop(MedianFinder *mf, bool *ok) {
    if (mf->sizeLow == 0) { if (ok) *ok = false; return 0; }
    if (ok) *ok = true;
    int ret = mf->low[0];
    mf->low[0] = mf->low[mf->sizeLow - 1];
    mf->sizeLow--;
    int i = 0;
    while (1) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l >= mf->sizeLow) break;
        int largest = i;
        if (mf->low[l] > mf->low[largest]) largest = l;
        if (r < mf->sizeLow && mf->low[r] > mf->low[largest]) largest = r;
        if (largest == i) break;
        swapInt(&mf->low[i], &mf->low[largest]);
        i = largest;
    }
    return ret;
}

/* Min-heap operations (upper half) */
static bool minHeapPush(MedianFinder *mf, int val) {
    if (!ensureCapacity(&mf->high, &mf->capHigh, mf->sizeHigh + 1)) return false;
    int i = mf->sizeHigh++;
    mf->high[i] = val;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (mf->high[p] <= mf->high[i]) break;
        swapInt(&mf->high[p], &mf->high[i]);
        i = p;
    }
    return true;
}

static int minHeapTop(const MedianFinder *mf) {
    if (mf->sizeHigh == 0) return 0;
    return mf->high[0];
}

static int minHeapPop(MedianFinder *mf, bool *ok) {
    if (mf->sizeHigh == 0) { if (ok) *ok = false; return 0; }
    if (ok) *ok = true;
    int ret = mf->high[0];
    mf->high[0] = mf->high[mf->sizeHigh - 1];
    mf->sizeHigh--;
    int i = 0;
    while (1) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l >= mf->sizeHigh) break;
        int smallest = i;
        if (mf->high[l] < mf->high[smallest]) smallest = l;
        if (r < mf->sizeHigh && mf->high[r] < mf->high[smallest]) smallest = r;
        if (smallest == i) break;
        swapInt(&mf->high[i], &mf->high[smallest]);
        i = smallest;
    }
    return ret;
}

static void rebalance(MedianFinder *mf) {
    if (mf->sizeLow > mf->sizeHigh + 1) {
        bool ok = false;
        int moved = maxHeapPop(mf, &ok);
        if (ok) minHeapPush(mf, moved);
    } else if (mf->sizeHigh > mf->sizeLow) {
        bool ok = false;
        int moved = minHeapPop(mf, &ok);
        if (ok) maxHeapPush(mf, moved);
    }
}

MedianFinder* medianFinderCreate(void) {
    MedianFinder *mf = (MedianFinder*)calloc(1, sizeof(MedianFinder));
    if (!mf) return NULL;
    mf->low = NULL; mf->high = NULL;
    mf->sizeLow = mf->sizeHigh = 0;
    mf->capLow = mf->capHigh = 0;
    return mf;
}

void medianFinderFree(MedianFinder *mf) {
    if (!mf) return;
    free(mf->low);
    free(mf->high);
    free(mf);
}

bool medianFinderAddNum(MedianFinder *mf, int num) {
    if (!mf) return false;
    if (num < -100000 || num > 100000) {
        return false; // fail closed
    }
    if (mf->sizeLow == 0 || num <= maxHeapTop(mf)) {
        if (!maxHeapPush(mf, num)) return false;
    } else {
        if (!minHeapPush(mf, num)) return false;
    }
    rebalance(mf);
    return true;
}

double medianFinderFindMedian(const MedianFinder *mf, bool *ok) {
    if (!mf || (mf->sizeLow == 0 && mf->sizeHigh == 0)) {
        if (ok) *ok = false;
        errno = EINVAL;
        return 0.0;
    }
    if (ok) *ok = true;
    if (mf->sizeLow > mf->sizeHigh) {
        return (double)maxHeapTop(mf);
    } else {
        return ((double)maxHeapTop(mf) + (double)minHeapTop(mf)) / 2.0;
    }
}

int main(void) {
    // Test 1
    MedianFinder *mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    bool ok = false;
    double med = medianFinderFindMedian(mf1, &ok);
    if (ok) printf("%.6f\n", med); // 1.5
    medianFinderAddNum(mf1, 3);
    med = medianFinderFindMedian(mf1, &ok);
    if (ok) printf("%.6f\n", med); // 2.0
    medianFinderFree(mf1);

    // Test 2
    MedianFinder *mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, 2);
    medianFinderAddNum(mf2, 3);
    medianFinderAddNum(mf2, 4);
    med = medianFinderFindMedian(mf2, &ok);
    if (ok) printf("%.6f\n", med); // 3.0
    medianFinderFree(mf2);

    // Test 3
    MedianFinder *mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, 2);
    medianFinderAddNum(mf3, 3);
    med = medianFinderFindMedian(mf3, &ok);
    if (ok) printf("%.6f\n", med); // 2.5
    medianFinderFree(mf3);

    // Test 4
    MedianFinder *mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, -5);
    medianFinderAddNum(mf4, -1);
    medianFinderAddNum(mf4, -3);
    medianFinderAddNum(mf4, -4);
    med = medianFinderFindMedian(mf4, &ok);
    if (ok) printf("%.6f\n", med); // -3.5
    medianFinderFree(mf4);

    // Test 5
    MedianFinder *mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 5);
    medianFinderAddNum(mf5, 10);
    medianFinderAddNum(mf5, -1);
    medianFinderAddNum(mf5, 20);
    medianFinderAddNum(mf5, 3);
    med = medianFinderFindMedian(mf5, &ok);
    if (ok) printf("%.6f\n", med); // 5.0
    medianFinderFree(mf5);

    return 0;
}