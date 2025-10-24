#include <stdio.h>
#include <stddef.h>

// Define a struct with bit fields.
// The compiler will attempt to pack these into a smaller memory footprint.
typedef struct {
    // field_name : bit_width
    unsigned int day   : 5;  // 5 bits for day (0-31)
    unsigned int month : 4;  // 4 bits for month (0-15)
    unsigned int year  : 7;  // 7 bits for year (0-127), offset from 2000
} Date;

// Function to print the date stored in the bit field struct
void printDate(Date d) {
    // The size of the struct is implementation-defined due to potential padding
    // by the compiler for alignment purposes (often padded to the size of an int).
    printf("Date: %02u/%02u/%u (Struct size: %zu bytes)\n",
           d.month, d.day, d.year + 2000, sizeof(Date));
}

int main() {
    printf("C Test Cases (Native Bit Fields):\n");

    // Test Case 1
    Date date1 = {1, 1, 23};
    printDate(date1);

    // Test Case 2
    Date date2 = {31, 12, 99};
    printDate(date2);

    // Test Case 3
    Date date3 = {29, 2, 24};
    printDate(date3);

    // Test Case 4
    Date date4 = {15, 8, 47};
    printDate(date4);

    // Test Case 5
    Date date5 = {4, 7, 76};
    printDate(date5);

    return 0;
}