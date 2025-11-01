
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

void generateAndCalculate() {
    // Generate first random integer between 10 and 1000
    int num1 = rand() % 991 + 10;
    
    // Generate second random integer between 10 and 1000
    int num2 = rand() % 991 + 10;
    
    // Calculate the difference between the two integers
    int difference = num1 - num2;
    
    // Print the results
    cout << "First number: " << num1 << endl;
    cout << "Second number: " << num2 << endl;
    cout << "Difference: " << difference << endl;
}

int main() {
    srand(time(0));
    
    cout << "Test Case 1:" << endl;
    generateAndCalculate();
    cout << endl;
    
    cout << "Test Case 2:" << endl;
    generateAndCalculate();
    cout << endl;
    
    cout << "Test Case 3:" << endl;
    generateAndCalculate();
    cout << endl;
    
    cout << "Test Case 4:" << endl;
    generateAndCalculate();
    cout << endl;
    
    cout << "Test Case 5:" << endl;
    generateAndCalculate();
    
    return 0;
}
