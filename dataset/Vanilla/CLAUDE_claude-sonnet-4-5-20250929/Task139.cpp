
#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

double calculateArea(double radius) {
    return M_PI * radius * radius;
}

int main() {
    cout << fixed << setprecision(6);
    
    // Test cases
    cout << "Test Case 1 - Radius: 5.0" << endl;
    cout << "Area: " << calculateArea(5.0) << endl;
    cout << endl;
    
    cout << "Test Case 2 - Radius: 10.5" << endl;
    cout << "Area: " << calculateArea(10.5) << endl;
    cout << endl;
    
    cout << "Test Case 3 - Radius: 1.0" << endl;
    cout << "Area: " << calculateArea(1.0) << endl;
    cout << endl;
    
    cout << "Test Case 4 - Radius: 7.25" << endl;
    cout << "Area: " << calculateArea(7.25) << endl;
    cout << endl;
    
    cout << "Test Case 5 - Radius: 15.8" << endl;
    cout << "Area: " << calculateArea(15.8) << endl;
    
    return 0;
}
