#include <iostream>
using namespace std;

int nb_year(int p0, double percent, int aug, int p) {
    int years = 0;
    while (p0 < p) {
        p0 += static_cast<int>(p0 * percent / 100.0) + aug;
        years++;
    }
    return years;
}

int main() {
    cout << nb_year(1000, 2, 50, 1200) << endl;            // Expected: 3
    cout << nb_year(1500, 5, 100, 5000) << endl;           // Expected: 15
    cout << nb_year(1500000, 2.5, 10000, 2000000) << endl; // Expected: 10
    cout << nb_year(1500, 0, 100, 2000) << endl;           // Expected: 5
    cout << nb_year(100, 5, 0, 101) << endl;               // Expected: 1
    return 0;
}