#include <iostream>
#include <string>
#include <algorithm> // For std::reverse
#include <vector>

/**
 * @brief Creates a copy of a std::string.
 * Using std::string is inherently safe from buffer overflows.
 * @param source The string to copy.
 * @return A new std::string identical to the source.
 */
std::string stringCopy(const std::string& source) {
    return source; // std::string handles memory copying safely.
}

/**
 * @brief Concatenates two std::strings.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A new std::string containing s1 followed by s2.
 */
std::string stringConcat(const std::string& s1, const std::string& s2) {
    return s1 + s2; // std::string's operator+ is safe.
}

/**
 * @brief Reverses a std::string.
 * @param source The string to reverse.
 * @return A new, reversed std::string.
 */
std::string stringReverse(const std::string& source) {
    std::string reversed = source;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

int main() {
    // Test Case 1: Basic operations
    std::cout << "--- Test Case 1: Basic Operations ---" << std::endl;
    std::string str1 = "hello";
    std::string str2 = "world";

    std::string copiedStr = stringCopy(str1);
    std::cout << "Original: " << str1 << ", Copied: " << copiedStr << std::endl;

    std::string concatenatedStr = stringConcat(str1, str2);
    std::cout << "Concatenated: \"" << str1 << "\" + \"" << str2 << "\" = \"" << concatenatedStr << "\"" << std::endl;

    std::string reversedStr = stringReverse("CPP");
    std::cout << "Original: CPP, Reversed: " << reversedStr << std::endl;
    std::cout << std::endl;

    // Test Case 2: Empty strings
    std::cout << "--- Test Case 2: Empty Strings ---" << std::endl;
    std::string emptyStr = "";
    std::string textStr = "test";

    copiedStr = stringCopy(emptyStr);
    std::cout << "Original: \"\", Copied: \"" << copiedStr << "\"" << std::endl;

    concatenatedStr = stringConcat(emptyStr, textStr);
    std::cout << "Concatenated: \"\" + \"" << textStr << "\" = \"" << concatenatedStr << "\"" << std::endl;

    reversedStr = stringReverse(emptyStr);
    std::cout << "Original: \"\", Reversed: \"" << reversedStr << "\"" << std::endl;
    std::cout << std::endl;

    // Test Case 3: Long strings (std::string handles this automatically)
    std::cout << "--- Test Case 3: Long Strings ---" << std::endl;
    std::string longStr1(50, 'a');
    std::string longStr2(50, 'b');
    concatenatedStr = stringConcat(longStr1, longStr2);
    std::cout << "Concatenated two long strings. Result length: " << concatenatedStr.length() << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Strings with spaces and special characters
    std::cout << "--- Test Case 4: Special Characters ---" << std::endl;
    std::string specialStr = " A B C!@#123 ";
    copiedStr = stringCopy(specialStr);
    std::cout << "Original: \"" << specialStr << "\", Copied: \"" << copiedStr << "\"" << std::endl;

    concatenatedStr = stringConcat(specialStr, "end");
    std::cout << "Concatenated: \"" << specialStr << "\" + \"end\" = \"" << concatenatedStr << "\"" << std::endl;

    reversedStr = stringReverse(specialStr);
    std::cout << "Original: \"" << specialStr << "\", Reversed: \"" << reversedStr << "\"" << std::endl;
    std::cout << std::endl;

    // Test Case 5: Single character string
    std::cout << "--- Test Case 5: Single Character String ---" << std::endl;
    std::string singleCharStr = "a";
    copiedStr = stringCopy(singleCharStr);
    std::cout << "Original: " << singleCharStr << ", Copied: " << copiedStr << std::endl;

    concatenatedStr = stringConcat(singleCharStr, "b");
    std::cout << "Concatenated: \"" << singleCharStr << "\" + \"b\" = \"" << concatenatedStr << "\"" << std::endl;

    reversedStr = stringReverse(singleCharStr);
    std::cout << "Original: " << singleCharStr << ", Reversed: " << reversedStr << std::endl;

    return 0;
}