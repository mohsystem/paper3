
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

using namespace std;

void processInput(int argc, char* argv[]) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        cout << "Error: Please provide exactly one argument" << endl;
        exit(1);
    }
    
    // Get the argument from command line
    string input = argv[1];
    
    // Print the argument
    cout << "Original input: " << input << endl;
    
    // Trim the trailing whitespace
    size_t end = input.find_last_not_of(" \\t\\n\\r");
    if (end != string::npos) {
        input = input.substr(0, end + 1);
    }
    
    // Print the argument again
    cout << "Trimmed input: " << input << endl;
    
    // Exit the program
    exit(0);
}

int main() {
    // Test case 1: Normal string with trailing spaces
    cout << "Test Case 1:" << endl;
    char* args1[] = {(char*)"program", (char*)"Hello World   "};
    processInput(2, args1);
    
    cout << "\\nTest Case 2:" << endl;
    char* args2[] = {(char*)"program", (char*)"Test123\\t\\t"};
    processInput(2, args2);
    
    cout << "\\nTest Case 3:" << endl;
    char* args3[] = {(char*)"program", (char*)"NoTrailingSpace"};
    processInput(2, args3);
    
    cout << "\\nTest Case 4:" << endl;
    char* args4[] = {(char*)"program", (char*)"Multiple   Spaces   "};
    processInput(2, args4);
    
    cout << "\\nTest Case 5:" << endl;
    char* args5[] = {(char*)"program", (char*)"   Leading and trailing   "};
    processInput(2, args5);
    
    return 0;
}
