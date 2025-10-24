#include <stdio.h>
#include <limits.h>
#include <stddef.h>

typedef struct {
    long long on;
    long long off;
} Pair;

typedef struct {
    int ok;            /* 1 for success, 0 for error */
    long long value;   /* valid when ok == 1 */
    const char* error; /* error message when ok == 0 */
} Result;

static int will_add_overflow_ll(long long a, long long b) {
    if (b > 0 && a > LLONG_MAX - b) return 1;
    if (b < 0 && a < LLONG_MIN - b) return 1;
    return 0;
}

/* Computes the number of people left on the bus after processing all stops.
   Returns ok=0 on validation failure (negative values, off > onboard, or overflow). */
Result compute_people(const Pair* stops, size_t count) {
    Result r;
    r.ok = 1;
    r.value = 0;
    r.error = NULL;

    if (count == 0) {
        r.value = 0;
        return r;
    }
    if (stops == NULL) {
        r.ok = 0;
        r.error = "Null pointer for stops with nonzero count";
        return r;
    }

    long long total = 0;
    for (size_t i = 0; i < count; ++i) {
        long long on = stops[i].on;
        long long off = stops[i].off;

        if (on < 0 || off < 0) {
            r.ok = 0;
            r.error = "Negative value detected";
            return r;
        }
        if (will_add_overflow_ll(total, on)) {
            r.ok = 0;
            r.error = "Overflow when adding boarding";
            return r;
        }
        total += on;

        if (off > total) {
            r.ok = 0;
            r.error = "More people getting off than available on bus";
            return r;
        }
        total -= off;
    }

    r.value = total;
    return r;
}

int main(void) {
    /* 1) Typical example */
    Pair t1[] = { {10,0}, {3,5}, {5,8} }; /* Expected: 5 */

    /* 2) Another typical sequence */
    Pair t2[] = { {3,0}, {9,1}, {4,10}, {12,2} }; /* Expected: 15 */

    /* 3) Empty list of stops */
    Pair* t3 = NULL; /* Expected: 0 */

    /* 4) Invalid: more off than on at a stop */
    Pair t4[] = { {5,0}, {2,10} }; /* Expected: error */

    /* 5) Large numbers within 64-bit limits */
    Pair t5[] = { {2000000000LL,0LL}, {2000000000LL,0LL}, {2000000000LL,0LL} }; /* Expected: 6000000000 */

    Result r1 = compute_people(t1, sizeof(t1)/sizeof(t1[0]));
    Result r2 = compute_people(t2, sizeof(t2)/sizeof(t2[0]));
    Result r3 = compute_people(t3, 0);
    Result r4 = compute_people(t4, sizeof(t4)/sizeof(t4[0]));
    Result r5 = compute_people(t5, sizeof(t5)/sizeof(t5[0]));

    printf("Test 1: ");
    if (r1.ok) printf("People left on bus = %lld\n", r1.value);
    else printf("Error: %s\n", r1.error ? r1.error : "Unknown error");

    printf("Test 2: ");
    if (r2.ok) printf("People left on bus = %lld\n", r2.value);
    else printf("Error: %s\n", r2.error ? r2.error : "Unknown error");

    printf("Test 3: ");
    if (r3.ok) printf("People left on bus = %lld\n", r3.value);
    else printf("Error: %s\n", r3.error ? r3.error : "Unknown error");

    printf("Test 4: ");
    if (r4.ok) printf("People left on bus = %lld\n", r4.value);
    else printf("Error: %s\n", r4.error ? r4.error : "Unknown error");

    printf("Test 5: ");
    if (r5.ok) printf("People left on bus = %lld\n", r5.value);
    else printf("Error: %s\n", r5.error ? r5.error : "Unknown error");

    return 0;
}