
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

using namespace std;

// String copy operation
string copyString(const string& source) {
    return string(source);
}

// String concatenation operation
string concatenateStrings(const string& str1, const string& str2) {
    return str1 + str2;
}

// String reverse operation
string reverseString(const string& input) {
    string result = input;
    reverse(result.begin(), result.end());
    return result;
}

// String uppercase operation
string toUpperCaseString(const string& input) {
    string result = input;
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// String lowercase operation
string toLowerCaseString(const string& input) {
    string result = input;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// String substring operation
string getSubstring(const string& input, int start, int end) {
    if (start < 0 || end > (int)input.length() || start > end) {
        return "";
    }
    return input.substr(start, end - start);
}

// String replace operation
string replaceString(const string& input, const string& target, const string& replacement) {
    string result = input;
    size_t pos = 0;
    while ((pos = result.find(target, pos)) != string::npos) {
        result.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }
    return result;
}

// String length operation
int getStringLength(const string& input) {
    return input.length();
}

int main() {
    // Test Case 1: Copy string
    cout << "Test Case 1 - Copy String:" << endl;
    string original = "Hello World";
    string copied = copyString(original);
    cout << "Original: " << original << endl;
    cout << "Copied: " << copied << endl;
    cout << endl;
    
    // Test Case 2: Concatenate strings
    cout << "Test Case 2 - Concatenate Strings:" << endl;
    string str1 = "Hello";
    string str2 = " World";
    string concatenated = concatenateStrings(str1, str2);
    cout << "String 1: " << str1 << endl;
    cout << "String 2: " << str2 << endl;
    cout << "Concatenated: " << concatenated << endl;
    cout << endl;
    
    // Test Case 3: Reverse string
    cout << "Test Case 3 - Reverse String:" << endl;
    string text = "Programming";
    string reversed = reverseString(text);
    cout << "Original: " << text << endl;
    cout << "Reversed: " << reversed << endl;
    cout << endl;
    
    // Test Case 4: Case conversion and substring
    cout << "Test Case 4 - Case Conversion and Substring:" << endl;
    string message = "Secure Coding Practice";
    cout << "Original: " << message << endl;
    cout << "Uppercase: " << toUpperCaseString(message) << endl;
    cout << "Lowercase: " << toLowerCaseString(message) << endl;
    cout << "Substring (0-6): " << getSubstring(message, 0, 6) << endl;
    cout << endl;
    
    // Test Case 5: Replace and length
    cout << "Test Case 5 - Replace and Length:" << endl;
    string sentence = "Java is great. Java is powerful.";
    string replaced = replaceString(sentence, "Java", "Python");
    cout << "Original: " << sentence << endl;
    cout << "Replaced: " << replaced << endl;
    cout << "Original Length: " << getStringLength(sentence) << endl;
    cout << "Replaced Length: " << getStringLength(replaced) << endl;
    
    return 0;
}
