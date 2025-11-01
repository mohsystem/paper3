#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
using namespace std;

string simulateMemory(const string& input) {
    size_t n = input.size();
    char* chunk1 = (char*)malloc(n + 1);
    char* chunk2 = (char*)malloc(n + 1);

    memcpy(chunk1, input.c_str(), n + 1);
    memcpy(chunk2, chunk1, n + 1);

    cout << chunk2 << endl;

    free(chunk1);

    cout << chunk2 << endl;

    string result = input + "\n" + input;

    free(chunk2);

    return result;
}

int main() {
    string tests[5] = {
        "Test1",
        "Second",
        "A longer string for memory copy",
        "1234567890",
        "EdgeCase!"
    };
    for (const auto& t : tests) {
        simulateMemory(t);
    }
    return 0;
}