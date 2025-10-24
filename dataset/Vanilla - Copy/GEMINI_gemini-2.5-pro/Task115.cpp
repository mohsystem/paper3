#include <iostream>
#include <iomanip>

// Define a struct with bit fields.
// The compiler will pack these fields into a single machine word if possible.
struct Date {
    // Field : bit_width
    unsigned int day   : 5;  // 5 bits for day (0-31)
    unsigned int month : 4;  // 4 bits for month (0-15)
    unsigned int year  : 7;  // 7 bits for year (0-127), offset from 2000
};

// Function to print the date stored in the bit field struct
void printDate(const Date& d) {
    // The size of the struct is implementation-defined. The compiler may add
    // padding to align the struct to word boundaries (e.g., 4 bytes for an int).
    std::cout << "Date: " << std::setfill('0') << std::setw(2) << d.month
              << "/" << std::setfill('0') << std::setw(2) << d.day
              << "/" << d.year + 2000
              << " (Struct size: " << sizeof(Date) << " bytes)" << std::endl;
}

int main() {
    std::cout << "CPP Test Cases (Native Bit Fields):" << std::endl;

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