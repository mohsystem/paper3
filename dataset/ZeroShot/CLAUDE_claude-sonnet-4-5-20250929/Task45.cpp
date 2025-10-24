
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cctype>

using namespace std;

class Task45 {
public:
    static string performOperation(const string& input, const string& operation) {
        try {
            if (input.empty() && operation != "length" && operation != "wordcount") {
                throw invalid_argument("Input cannot be empty for this operation");
            }
            
            string trimmedInput = trim(input);
            string trimmedOp = toLowerCase(trim(operation));
            
            if (trimmedOp == "uppercase") {
                string result = trimmedInput;
                transform(result.begin(), result.end(), result.begin(), ::toupper);
                return result;
            }
            else if (trimmedOp == "lowercase") {
                string result = trimmedInput;
                transform(result.begin(), result.end(), result.begin(), ::tolower);
                return result;
            }
            else if (trimmedOp == "reverse") {
                string result = trimmedInput;
                reverse(result.begin(), result.end());
                return result;
            }
            else if (trimmedOp == "length") {
                return to_string(trimmedInput.length());
            }
            else if (trimmedOp == "wordcount") {
                if (trimmedInput.empty()) return "0";
                int count = 0;
                istringstream iss(trimmedInput);
                string word;
                while (iss >> word) count++;
                return to_string(count);
            }
            else if (trimmedOp == "number") {
                try {
                    double num = stod(trimmedInput);
                    return to_string(num * 2);
                } catch (const exception&) {
                    throw invalid_argument("Input is not a valid number for 'number' operation");
                }
            }
            else {
                throw invalid_argument("Unknown operation: " + trimmedOp);
            }
        }
        catch (const invalid_argument& e) {
            return string("Error: ") + e.what();
        }
        catch (const exception& e) {
            return string("Error: Unexpected error occurred - ") + e.what();
        }
    }
    
private:
    static string trim(const string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
    static string toLowerCase(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
};

int main() {
    cout << "=== Test Cases ===" << endl << endl;
    
    // Test case 1: Uppercase operation
    string result1 = Task45::performOperation("hello world", "uppercase");
    cout << "Test 1 - Uppercase: " << result1 << endl;
    
    // Test case 2: Reverse operation
    string result2 = Task45::performOperation("Java Programming", "reverse");
    cout << "Test 2 - Reverse: " << result2 << endl;
    
    // Test case 3: Word count operation
    string result3 = Task45::performOperation("This is a test sentence", "wordcount");
    cout << "Test 3 - Word Count: " << result3 << endl;
    
    // Test case 4: Number operation with valid input
    string result4 = Task45::performOperation("42.5", "number");
    cout << "Test 4 - Number Operation: " << result4 << endl;
    
    // Test case 5: Error handling - invalid operation
    string result5 = Task45::performOperation("test", "invalid_op");
    cout << "Test 5 - Invalid Operation: " << result5 << endl;
    
    return 0;
}
