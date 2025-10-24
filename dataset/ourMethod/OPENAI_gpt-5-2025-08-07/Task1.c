#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

typedef long long ll;

typedef struct {
    int on;
    int off;
} StopPair;

static int will_add_overflow_ll(ll a, ll b) {
    if (b > 0 && a > LLONG_MAX - b) return 1;
    if (b < 0 && a < LLONG_MIN - b) return 1;
    return 0;
}

// Computes the number of people remaining on the bus after the last stop.
// Returns -1 on invalid input (negative values, underflow, or overflow).
ll people_on_bus(const StopPair* stops, size_t n) {
    if (stops == NULL && n > 0) {
        return -1;
    }

    ll occupants = 0;
    for (size_t i = 0; i < n; ++i) {
        int on = stops[i].on;
        int off = stops[i].off;

        if (on < 0 || off < 0) {
            return -1; // invalid numbers
        }

        // Check addition overflow
        if (will_add_overflow_ll(occupants, (ll)on)) {
            return -1;
        }
        occupants += (ll)on;

        // Validate we don't go negative (more off than current occupants)
        if ((ll)off > occupants) {
            return -1;
        }

        occupants -= (ll)off;
    }
    return occupants;
}

int main(void) {
    // Test case 1
    StopPair t1[] = { {10,0}, {3,5}, {5,8} };
    printf("Test 1: %lld\n", people_on_bus(t1, sizeof(t1)/sizeof(t1[0]))); // Expected: 5

    // Test case 2
    StopPair t2[] = { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} };
    printf("Test 2: %lld\n", people_on_bus(t2, sizeof(t2)/sizeof(t2[0]))); // Expected: 17

    // Test case 3 - empty array
    StopPair t3[] = { };
    printf("Test 3: %lld\n", people_on_bus(t3, 0)); // Expected: 0

    // Test case 4 - invalid: negative off
    StopPair t4[] = { {5,0}, {3,-1} };
    printf("Test 4: %lld\n", people_on_bus(t4, sizeof(t4)/sizeof(t4[0]))); // Expected: -1

    // Test case 5 - invalid: more off than current occupants
    StopPair t5[] = { {0,0}, {0,1} };
    printf("Test 5: %lld\n", people_on_bus(t5, sizeof(t5)/sizeof(t5[0]))); // Expected: -1

    return 0;
}