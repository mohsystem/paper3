#include <iostream>
#include <string>
#include <vector>

// The argument is a std::string, so it is already checked to be a string type.
void processString(std::string& s) {
    // The for loop trims the trailing whitespace (space, tab) in the string.
    int i = s.length() - 1;
    for (; i >= 0; --i) {
        if (s[i] != ' ' && s[i] != '\t') {
            break;
        }
    }
    s.resize(i + 1);

    // The second for loop trims the newlines.
    int j = s.length() - 1;
    for (; j >= 0; --j) {
        if (s[j] != '\n' && s[j] != '\r') {
            break;
        }
    }
    // The last line null terminates the string.
    s.resize(j + 1);
}

void print_escaped(const std::string& s) {
    std::cout << "\"";
    for (char c : s) {
        switch (c) {
            case '\n': std::cout << "\\n"; break;
            case '\r': std::cout << "\\r"; break;
            case '\t': std::cout << "\\t"; break;
            default:   std::cout << c; break;
        }
    }
    std::cout << "\"";
}

int main(int argc, char* argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    if (argc == 2) {
        // The argument argv[1] is a C-style string. We convert it to std::string.
        std::string arg(argv[1]);
        std::cout << "Processing command-line argument:" << std::endl;
        std::cout << "Original:  ";
        print_escaped(arg);
        std::cout << std::endl;

        processString(arg); 

        std::cout << "Processed: ";
        print_escaped(arg);
        std::cout << std::endl;
        std::cout << "---------------------------------" << std::endl;
    } else {
        std::cout << "Incorrect argument count. Running test cases instead." << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    std::vector<std::string> testCases = {
        "  hello world   \t\n\r",
        "test case 2\t\t",
        "no trailing whitespace",
        "only newlines\n\n\r\n",
        "   \t \n \r "
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::string currentTest = testCases[i];
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Original:  ";
        print_escaped(currentTest);
        std::cout << std::endl;

        processString(currentTest);

        std::cout << "Processed: ";
        print_escaped(currentTest);
        std::cout << std::endl << std::endl;
    }

    return 0;
}