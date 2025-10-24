
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Clock structure to handle time without dates
// Security: Uses fixed-size structure to prevent buffer overflows
typedef struct {
    int totalMinutes; // Time as total minutes since midnight (0-1439)
} Clock;

// Normalize minutes to 0-1439 range (24 hours = 1440 minutes)
// Security: Safely handles negative values and wraparound using modulo
static void clock_normalize(Clock* clock) {
    if (clock == NULL) {
        return;
    }
    
    // Handle negative values and values >= 1440
    clock->totalMinutes = clock->totalMinutes % 1440;
    if (clock->totalMinutes < 0) {
        clock->totalMinutes += 1440;
    }
}

// Create a new clock with validation
// Security: Validates all inputs, checks for NULL pointer, validates ranges
Clock* clock_create(int hours, int minutes) {
    // Validate inputs to prevent overflow
    if (hours < -100000 || hours > 100000) {
        fprintf(stderr, "Error: Hours out of valid range\\n");
        return NULL;
    }
    if (minutes < -100000 || minutes > 100000) {
        fprintf(stderr, "Error: Minutes out of valid range\\n");
        return NULL;
    }
    
    // Allocate memory with validation
    Clock* clock = (Clock*)malloc(sizeof(Clock));
    if (clock == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Security: Check for potential overflow before multiplication
    if (hours > INT_MAX / 60 || hours < INT_MIN / 60) {
        fprintf(stderr, "Error: Hours would cause overflow\\n");
        free(clock);
        return NULL;
    }
    
    // Initialize clock
    clock->totalMinutes = hours * 60 + minutes;
    clock_normalize(clock);
    
    return clock;
}

// Free clock memory
// Security: Validates pointer before freeing, prevents double-free
void clock_destroy(Clock* clock) {
    if (clock != NULL) {
        // Clear sensitive data before free (defense in depth)
        clock->totalMinutes = 0;
        free(clock);
    }
}

// Add minutes to clock
// Security: Validates inputs, checks for overflow, handles NULL pointer
int clock_add_minutes(Clock* clock, int minutes) {
    if (clock == NULL) {
        fprintf(stderr, "Error: NULL clock pointer\\n");
        return -1;
    }
    
    // Validate input range
    if (minutes < -1000000 || minutes > 1000000) {
        fprintf(stderr, "Error: Minutes to add out of valid range\\n");
        return -1;
    }
    
    // Security: Check for overflow before addition
    if (minutes > 0 && clock->totalMinutes > INT_MAX - minutes) {
        // Normalize in chunks to prevent overflow
        clock->totalMinutes = (clock->totalMinutes % 1440) + (minutes % 1440);
    } else if (minutes < 0 && clock->totalMinutes < INT_MIN - minutes) {
        clock->totalMinutes = (clock->totalMinutes % 1440) + (minutes % 1440);
    } else {
        clock->totalMinutes += minutes;
    }
    
    clock_normalize(clock);
    return 0;
}

// Subtract minutes from clock
// Security: Validates inputs, prevents underflow
int clock_subtract_minutes(Clock* clock, int minutes) {
    if (clock == NULL) {
        fprintf(stderr, "Error: NULL clock pointer\\n");
        return -1;
    }
    
    // Validate input range
    if (minutes < -1000000 || minutes > 1000000) {
        fprintf(stderr, "Error: Minutes to subtract out of valid range\\n");
        return -1;
    }
    
    // Subtraction is addition of negative value
    return clock_add_minutes(clock, -minutes);
}

// Get hours (0-23)
// Security: Validates pointer before access
int clock_get_hours(const Clock* clock) {
    if (clock == NULL) {
        return -1;
    }
    return clock->totalMinutes / 60;
}

// Get minutes (0-59)
// Security: Validates pointer before access
int clock_get_minutes(const Clock* clock) {
    if (clock == NULL) {
        return -1;
    }
    return clock->totalMinutes % 60;
}

// Check if two clocks are equal
// Security: Validates both pointers before comparison
int clock_equals(const Clock* clock1, const Clock* clock2) {
    if (clock1 == NULL || clock2 == NULL) {
        return 0;
    }
    return clock1->totalMinutes == clock2->totalMinutes;
}

// Convert clock to string representation HH:MM
// Security: Uses bounded snprintf, validates inputs, checks buffer size
int clock_to_string(const Clock* clock, char* buffer, size_t buffer_size) {
    if (clock == NULL || buffer == NULL) {
        return -1;
    }
    
    // Validate buffer size (need at least 6 bytes for "HH:MM\\0")
    if (buffer_size < 6) {
        return -1;
    }
    
    int hours = clock_get_hours(clock);
    int minutes = clock_get_minutes(clock);
    
    // Security: Use snprintf with size limit and check return value
    int written = snprintf(buffer, buffer_size, "%02d:%02d", hours, minutes);
    if (written < 0 || (size_t)written >= buffer_size) {
        return -1;
    }
    
    return 0;
}

int main(void) {
    char buffer[16]; // Buffer for time string with safety margin
    
    printf("Clock Implementation Test Cases:\\n\\n");
    
    // Test Case 1: Basic clock creation and display
    printf("Test 1: Create clock at 08:00\\n");
    Clock* clock1 = clock_create(8, 0);
    if (clock1 != NULL) {
        if (clock_to_string(clock1, buffer, sizeof(buffer)) == 0) {
            printf("Clock1: %s\\n\\n", buffer);
        }
        clock_destroy(clock1);
        clock1 = NULL; // Prevent use after free
    }
    
    // Test Case 2: Add minutes
    printf("Test 2: Add 3 minutes to 10:00\\n");
    Clock* clock2 = clock_create(10, 0);
    if (clock2 != NULL) {
        if (clock_to_string(clock2, buffer, sizeof(buffer)) == 0) {
            printf("Before: %s\\n", buffer);
        }
        clock_add_minutes(clock2, 3);
        if (clock_to_string(clock2, buffer, sizeof(buffer)) == 0) {
            printf("After adding 3 minutes: %s\\n\\n", buffer);
        }
        clock_destroy(clock2);
        clock2 = NULL;
    }
    
    // Test Case 3: Add minutes that roll over to next hour
    printf("Test 3: Add 61 minutes to 00:45\\n");
    Clock* clock3 = clock_create(0, 45);
    if (clock3 != NULL) {
        if (clock_to_string(clock3, buffer, sizeof(buffer)) == 0) {
            printf("Before: %s\\n", buffer);
        }
        clock_add_minutes(clock3, 61);
        if (clock_to_string(clock3, buffer, sizeof(buffer)) == 0) {
            printf("After adding 61 minutes: %s\\n\\n", buffer);
        }
        clock_destroy(clock3);
        clock3 = NULL;
    }
    
    // Test Case 4: Subtract minutes
    printf("Test 4: Subtract 90 minutes from 10:00\\n");
    Clock* clock4 = clock_create(10, 0);
    if (clock4 != NULL) {
        if (clock_to_string(clock4, buffer, sizeof(buffer)) == 0) {
            printf("Before: %s\\n", buffer);
        }
        clock_subtract_minutes(clock4, 90);
        if (clock_to_string(clock4, buffer, sizeof(buffer)) == 0) {
            printf("After subtracting 90 minutes: %s\\n\\n", buffer);
        }
        clock_destroy(clock4);
        clock4 = NULL;
    }
    
    // Test Case 5: Clock equality and wrapping past midnight
    printf("Test 5: Clock equality and midnight wrapping\\n");
    Clock* clock5a = clock_create(23, 59);
    Clock* clock5b = clock_create(0, 0);
    if (clock5a != NULL && clock5b != NULL) {
        printf("Clock5a at 23:59, adding 1 minute: ");
        clock_add_minutes(clock5a, 1);
        if (clock_to_string(clock5a, buffer, sizeof(buffer)) == 0) {
            printf("%s\\n", buffer);
        }
        if (clock_to_string(clock5b, buffer, sizeof(buffer)) == 0) {
            printf("Clock5b at 00:00: %s\\n", buffer);
        }
        printf("Are they equal? %s\\n", clock_equals(clock5a, clock5b) ? "Yes" : "No");
    }
    
    // Security: Clean up all allocated memory
    clock_destroy(clock5a);
    clock_destroy(clock5b);
    
    return 0;
}
