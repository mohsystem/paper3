#include <iostream>
#include <string>
#include <cctype>

/**
 * @brief Parses the JSON string to identify the root element's type.
 * 
 * This implementation performs a basic structural check by finding the first and
 * last non-whitespace characters to determine if the document is enclosed
 * in braces `{}` (Object) or brackets `[]` (Array).
 * 
 * @param jsonString The string representation of the JSON document.
 * @return "Object", "Array", or "Invalid" based on the root element.
 */
std::string findRootElement(const std::string& jsonString) {
    size_t first = jsonString.find_first_not_of(" \t\n\r\f\v");

    // If no non-whitespace character is found
    if (std::string::npos == first) {
        return "Invalid";
    }

    size_t last = jsonString.find_last_not_of(" \t\n\r\f\v");

    char firstChar = jsonString[first];
    char lastChar = jsonString[last];

    if (firstChar == '{' && lastChar == '}') {
        return "Object";
    } else if (firstChar == '[' && lastChar == ']') {
        return "Array";
    } else {
        return "Invalid";
    }
}

int main() {
    std::string testCases[] = {
        "  { \"name\": \"John\", \"age\": 30 }  ",
        "[ \"apple\", \"banana\", \"cherry\" ]",
        "   ",
        "this is not json",
        "{ \"incomplete\": \"json\""
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase << "\" -> Output: " << findRootElement(testCase) << std::endl;
    }

    // Test with an empty string
    std::cout << "Input: \"\" -> Output: " << findRootElement("") << std::endl;

    return 0;
}