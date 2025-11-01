
// WARNING: This code contains a critical security vulnerability (double-free)
// For educational purposes only - DO NOT USE IN PRODUCTION

#include <iostream>
#include <cstdlib>

using namespace std;

class Lse61 {
public:
    static void demonstrateVulnerability(int bufferSize, int freeCount) {
        // Allocate buffer
        char* buffer = (char*)malloc(bufferSize);
        
        if (buffer == nullptr) {
            cout << "Memory allocation failed!" << endl;
            return;
        }
        
        cout << "Buffer allocated with size: " << bufferSize 
             << " at address: " << (void*)buffer << endl;
        
        // VULNERABILITY: Freeing the same buffer multiple times
        for (int i = 0; i < freeCount; i++) {
            free(buffer);
            cout << "Buffer freed, iteration: " << (i + 1) << endl;
            // WARNING: After first free, subsequent frees cause undefined behavior
            // This can lead to crashes, corruption, or security vulnerabilities
        }
        
        cout << "WARNING: Double-free vulnerability demonstrated!" << endl;
    }
};

int main() {
    cout << "Test Case 1:" << endl;
    Lse61::demonstrateVulnerability(1024, 3);
    
    cout << "\\nTest Case 2:" << endl;
    Lse61::demonstrateVulnerability(2048, 5);
    
    cout << "\\nTest Case 3:" << endl;
    Lse61::demonstrateVulnerability(512, 2);
    
    cout << "\\nTest Case 4:" << endl;
    Lse61::demonstrateVulnerability(4096, 1);
    
    cout << "\\nTest Case 5:" << endl;
    Lse61::demonstrateVulnerability(256, 10);
    
    return 0;
}
