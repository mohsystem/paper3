
#include <iostream>
#include <string>
using namespace std;

string makeComplement(string dna) {
    string result = "";
    for (char c : dna) {
        switch (c) {
            case 'A':
                result += 'T';
                break;
            case 'T':
                result += 'A';
                break;
            case 'C':
                result += 'G';
                break;
            case 'G':
                result += 'C';
                break;
        }
    }
    return result;
}

int main() {
    // Test case 1
    cout << "Input: ATTGC, Output: " << makeComplement("ATTGC") << endl;
    
    // Test case 2
    cout << "Input: GTAT, Output: " << makeComplement("GTAT") << endl;
    
    // Test case 3
    cout << "Input: AAAA, Output: " << makeComplement("AAAA") << endl;
    
    // Test case 4
    cout << "Input: CGCG, Output: " << makeComplement("CGCG") << endl;
    
    // Test case 5
    cout << "Input: ATCGATCG, Output: " << makeComplement("ATCGATCG") << endl;
    
    return 0;
}
