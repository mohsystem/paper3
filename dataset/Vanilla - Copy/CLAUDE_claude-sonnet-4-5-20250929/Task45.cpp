
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>

using namespace std;

string toLowerCase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

string performOperation(string operation, double num1, double num2) {
    try {
        operation = toLowerCase(operation);
        
        if (operation == "add") {
            return to_string(num1 + num2);
        } else if (operation == "subtract") {
            return to_string(num1 - num2);
        } else if (operation == "multiply") {
            return to_string(num1 * num2);
        } else if (operation == "divide") {
            if (num2 == 0) {
                throw runtime_error("Division by zero");
            }
            return to_string(num1 / num2);
        } else if (operation == "modulo") {
            if (num2 == 0) {
                throw runtime_error("Modulo by zero");
            }
            return to_string(fmod(num1, num2));
        } else if (operation == "power") {
            return to_string(pow(num1, num2));
        } else {
            throw invalid_argument("Invalid operation: " + operation);
        }
    } catch (const runtime_error& e) {
        return string("Error: ") + e.what();
    } catch (const invalid_argument& e) {
        return string("Error: ") + e.what();
    } catch (const exception& e) {
        return string("Error: Unexpected error occurred - ") + e.what();
    }
}

string parseAndCalculate(string input) {
    try {
        if (input.empty()) {
            throw invalid_argument("Input cannot be empty");
        }
        
        istringstream iss(input);
        string operation;
        double num1, num2;
        
        if (!(iss >> operation >> num1 >> num2)) {
            throw invalid_argument("Invalid input format. Expected: <operation> <num1> <num2>");
        }
        
        return performOperation(operation, num1, num2);
        
    } catch (const invalid_argument& e) {
        return string("Error: ") + e.what();
    } catch (const exception& e) {
        return string("Error: Unexpected error - ") + e.what();
    }
}

string validateAndProcess(vector<string> inputs) {
    try {
        if (inputs.empty()) {
            throw runtime_error("Input array is null or empty");
        }
        
        stringstream results;
        for (size_t i = 0; i < inputs.size(); i++) {
            try {
                string result = parseAndCalculate(inputs[i]);
                results << "Input " << (i + 1) << ": " << result << "\\n";
            } catch (const exception& e) {
                results << "Input " << (i + 1) << ": Error - " << e.what() << "\\n";
            }
        }
        return results.str();
        
    } catch (const runtime_error& e) {
        return string("Error: ") + e.what();
    } catch (const exception& e) {
        return string("Error: Unexpected error in batch processing - ") + e.what();
    }
}

int main() {
    cout << "=== Test Case 1: Valid Addition ===" << endl;
    cout << parseAndCalculate("add 10 5") << endl;
    
    cout << "\\n=== Test Case 2: Division by Zero ===" << endl;
    cout << parseAndCalculate("divide 10 0") << endl;
    
    cout << "\\n=== Test Case 3: Invalid Operation ===" << endl;
    cout << parseAndCalculate("invalid 10 5") << endl;
    
    cout << "\\n=== Test Case 4: Invalid Number Format ===" << endl;
    cout << parseAndCalculate("multiply abc 5") << endl;
    
    cout << "\\n=== Test Case 5: Batch Processing ===" << endl;
    vector<string> batchInputs = {
        "add 100 50",
        "subtract 75 25",
        "multiply 8 7",
        "divide 100 4",
        "power 2 8"
    };
    cout << validateAndProcess(batchInputs) << endl;
    
    return 0;
}
