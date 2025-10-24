
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string concatenateStrings(const vector<string>& strings) {
    if (strings.empty()) {
        return "";
    }
    
    string result = "";
    for (const string& str : strings) {
        result += str;
    }
    return result;
}

int main() {
    // Test case 1: Multiple strings
    vector<string> test1 = {"Hello", " ", "World", "!"};
    cout << "Test 1: " << concatenateStrings(test1) << endl;
    
    // Test case 2: Empty vector
    vector<string> test2 = {};
    cout << "Test 2: " << concatenateStrings(test2) << endl;
    
    // Test case 3: Single string
    vector<string> test3 = {"SingleString"};
    cout << "Test 3: " << concatenateStrings(test3) << endl;
    
    // Test case 4: Strings with numbers and special characters
    vector<string> test4 = {"C++", "123", "@#$", "Test"};
    cout << "Test 4: " << concatenateStrings(test4) << endl;
    
    // Test case 5: Multiple words
    vector<string> test5 = {"This", " ", "is", " ", "a", " ", "test"};
    cout << "Test 5: " << concatenateStrings(test5) << endl;
    
    // Interactive user input
    cout << "\\nEnter number of strings to concatenate: ";
    int n;
    cin >> n;
    cin.ignore(); // consume newline
    
    vector<string> userStrings;
    cout << "Enter " << n << " strings:" << endl;
    for (int i = 0; i < n; i++) {
        string input;
        getline(cin, input);
        userStrings.push_back(input);
    }
    
    cout << "Result: " << concatenateStrings(userStrings) << endl;
    
    return 0;
}
