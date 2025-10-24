#include <iostream>
#include <vector>
using namespace std;

class ProductOfNumbers {
    vector<int> prefix;
public:
    ProductOfNumbers() {
        prefix.clear();
        prefix.push_back(1);
    }
    void add(int num) {
        if (num == 0) {
            prefix.clear();
            prefix.push_back(1);
        } else {
            prefix.push_back(prefix.back() * num);
        }
    }
    int getProduct(int k) {
        if (k >= (int)prefix.size()) return 0;
        int n = (int)prefix.size();
        return prefix[n - 1] / prefix[n - 1 - k];
    }
};

int main() {
    // Test case 1 (sample)
    ProductOfNumbers p1;
    p1.add(3); p1.add(0); p1.add(2); p1.add(5); p1.add(4);
    cout << p1.getProduct(2) << endl;
    cout << p1.getProduct(3) << endl;
    cout << p1.getProduct(4) << endl;
    p1.add(8);
    cout << p1.getProduct(2) << endl;

    // Test case 2
    ProductOfNumbers p2;
    p2.add(1); p2.add(2); p2.add(3); p2.add(4);
    cout << p2.getProduct(1) << endl;
    cout << p2.getProduct(4) << endl;

    // Test case 3
    ProductOfNumbers p3;
    p3.add(0); p3.add(0); p3.add(2); p3.add(3);
    cout << p3.getProduct(1) << endl;
    cout << p3.getProduct(3) << endl;

    // Test case 4
    ProductOfNumbers p4;
    p4.add(10); p4.add(10); p4.add(10);
    cout << p4.getProduct(2) << endl;
    cout << p4.getProduct(3) << endl;

    // Test case 5
    ProductOfNumbers p5;
    p5.add(100); p5.add(0); p5.add(100); p5.add(100);
    cout << p5.getProduct(1) << endl;
    cout << p5.getProduct(2) << endl;
    cout << p5.getProduct(3) << endl;

    return 0;
}