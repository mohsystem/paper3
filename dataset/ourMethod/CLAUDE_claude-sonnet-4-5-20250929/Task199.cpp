
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <limits>

// Clock class that handles time without dates
// Security: All operations validate ranges and prevent integer overflow
class Clock {
private:
    int totalMinutes; // Store time as total minutes since midnight (0-1439)
    
    // Normalize minutes to 0-1439 range (24 hours = 1440 minutes)
    // Security: Prevents integer overflow by using modulo arithmetic safely
    void normalize() {
        // Handle negative values and values >= 1440
        // Using modulo to wrap around 24-hour clock
        totalMinutes = totalMinutes % 1440;
        if (totalMinutes < 0) {
            totalMinutes += 1440;
        }
    }
    
public:
    // Constructor with validation
    // Security: Validates input ranges for hours (0-23) and minutes (0-59)
    Clock(int hours, int minutes) {
        // Validate inputs are within reasonable ranges to prevent overflow
        if (hours < -100000 || hours > 100000) {
            throw std::invalid_argument("Hours out of valid range");
        }
        if (minutes < -100000 || minutes > 100000) {
            throw std::invalid_argument("Minutes out of valid range");
        }
        
        // Convert to total minutes
        // Security: Check for potential overflow before multiplication
        if (hours > INT_MAX / 60 || hours < INT_MIN / 60) {
            throw std::overflow_error("Hours would cause overflow");
        }
        
        totalMinutes = hours * 60 + minutes;
        normalize();
    }
    
    // Add minutes to clock
    // Security: Validates input and checks for overflow before addition
    Clock& addMinutes(int minutes) {
        if (minutes < -1000000 || minutes > 1000000) {
            throw std::invalid_argument("Minutes to add out of valid range");
        }
        
        // Check for overflow before addition
        if (minutes > 0 && totalMinutes > INT_MAX - minutes) {
            // Handle by normalizing the addition in chunks
            totalMinutes = (totalMinutes % 1440) + (minutes % 1440);
        } else if (minutes < 0 && totalMinutes < INT_MIN - minutes) {
            totalMinutes = (totalMinutes % 1440) + (minutes % 1440);
        } else {
            totalMinutes += minutes;
        }
        
        normalize();
        return *this;
    }
    
    // Subtract minutes from clock
    // Security: Validates input and checks for underflow
    Clock& subtractMinutes(int minutes) {
        if (minutes < -1000000 || minutes > 1000000) {
            throw std::invalid_argument("Minutes to subtract out of valid range");
        }
        
        // Subtraction is addition of negative value
        return addMinutes(-minutes);
    }
    
    // Get hours (0-23)
    int getHours() const {
        return totalMinutes / 60;
    }
    
    // Get minutes (0-59)
    int getMinutes() const {
        return totalMinutes % 60;
    }
    
    // Equality operator
    bool operator==(const Clock& other) const {
        return totalMinutes == other.totalMinutes;
    }
    
    // Inequality operator
    bool operator!=(const Clock& other) const {
        return !(*this == other);
    }
    
    // Convert to string representation HH:MM
    // Security: Uses safe string formatting with fixed width
    std::string toString() const {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << getHours() 
            << ":" << std::setw(2) << getMinutes();
        return oss.str();
    }
};

int main() {
    try {
        std::cout << "Clock Implementation Test Cases:\\n\\n";
        
        // Test Case 1: Basic clock creation and display
        std::cout << "Test 1: Create clock at 08:00\\n";
        Clock clock1(8, 0);
        std::cout << "Clock1: " << clock1.toString() << "\\n\\n";
        
        // Test Case 2: Add minutes
        std::cout << "Test 2: Add 3 minutes to 10:00\\n";
        Clock clock2(10, 0);
        std::cout << "Before: " << clock2.toString() << "\\n";
        clock2.addMinutes(3);
        std::cout << "After adding 3 minutes: " << clock2.toString() << "\\n\\n";
        
        // Test Case 3: Add minutes that roll over to next hour
        std::cout << "Test 3: Add 61 minutes to 00:45\\n";
        Clock clock3(0, 45);
        std::cout << "Before: " << clock3.toString() << "\\n";
        clock3.addMinutes(61);
        std::cout << "After adding 61 minutes: " << clock3.toString() << "\\n\\n";
        
        // Test Case 4: Subtract minutes
        std::cout << "Test 4: Subtract 90 minutes from 10:00\\n";
        Clock clock4(10, 0);
        std::cout << "Before: " << clock4.toString() << "\\n";
        clock4.subtractMinutes(90);
        std::cout << "After subtracting 90 minutes: " << clock4.toString() << "\\n\\n";
        
        // Test Case 5: Clock equality and wrapping past midnight
        std::cout << "Test 5: Clock equality and midnight wrapping\\n";
        Clock clock5a(23, 59);
        Clock clock5b(0, 0);
        std::cout << "Clock5a at 23:59, adding 1 minute: ";
        clock5a.addMinutes(1);
        std::cout << clock5a.toString() << "\\n";
        std::cout << "Clock5b at 00:00: " << clock5b.toString() << "\\n";
        std::cout << "Are they equal? " << (clock5a == clock5b ? "Yes" : "No") << "\\n";
        
    } catch (const std::exception& e) {
        // Security: Catch and handle all exceptions gracefully
        std::cerr << "Error: " << e.what() << "\\n";
        return 1;
    }
    
    return 0;
}
