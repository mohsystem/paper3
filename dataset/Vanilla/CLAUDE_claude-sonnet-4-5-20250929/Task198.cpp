
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

// Simulating rand7() API
int rand7() {
    return rand() % 7 + 1;
}

int rand10() {
    while (true) {
        // Generate a number in range [1, 49]
        int row = rand7();
        int col = rand7();
        int num = (row - 1) * 7 + col;
        
        // Only use numbers 1-40 to ensure uniformity
        if (num <= 40) {
            return (num - 1) % 10 + 1;
        }
    }
}

int main() {
    srand(time(0));
    
    // Test case 1: n = 1
    cout << "Test 1 (n=1): [";
    for (int i = 0; i < 1; i++) {
        cout << rand10();
        if (i < 0) cout << ",";
    }
    cout << "]" << endl;
    
    // Test case 2: n = 2
    cout << "Test 2 (n=2): [";
    for (int i = 0; i < 2; i++) {
        cout << rand10();
        if (i < 1) cout << ",";
    }
    cout << "]" << endl;
    
    // Test case 3: n = 3
    cout << "Test 3 (n=3): [";
    for (int i = 0; i < 3; i++) {
        cout << rand10();
        if (i < 2) cout << ",";
    }
    cout << "]" << endl;
    
    // Test case 4: n = 5
    cout << "Test 4 (n=5): [";
    for (int i = 0; i < 5; i++) {
        cout << rand10();
        if (i < 4) cout << ",";
    }
    cout << "]" << endl;
    
    // Test case 5: n = 10
    cout << "Test 5 (n=10): [";
    for (int i = 0; i < 10; i++) {
        cout << rand10();
        if (i < 9) cout << ",";
    }
    cout << "]" << endl;
    
    return 0;
}
