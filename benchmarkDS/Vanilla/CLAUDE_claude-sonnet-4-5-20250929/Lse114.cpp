
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>

using namespace std;

class Lse114 {
public:
    static void executeCommand(const string& command) {
        cout << "Executing: " << command << endl;
        int result = system(command.c_str());
        if (result != 0) {
            cerr << "Command execution failed with code: " << result << endl;
        }
    }
    
    static void executeLsCommand(const string& directory) {
        char command[256];
        snprintf(command, sizeof(command), "ls %s", directory.c_str());
        executeCommand(command);
    }
};

int main() {
    // Test case 1: List current directory
    cout << "Test Case 1: Current directory" << endl;
    Lse114::executeLsCommand(".");
    
    // Test case 2: List home directory
    cout << "\\nTest Case 2: Home directory" << endl;
    Lse114::executeLsCommand("~");
    
    // Test case 3: List with wildcard
    cout << "\\nTest Case 3: List with wildcard" << endl;
    Lse114::executeLsCommand("*.cpp");
    
    // Test case 4: List parent directory
    cout << "\\nTest Case 4: Parent directory" << endl;
    Lse114::executeLsCommand("..");
    
    // Test case 5: List /tmp directory
    cout << "\\nTest Case 5: /tmp directory" << endl;
    Lse114::executeLsCommand("/tmp");
    
    return 0;
}
