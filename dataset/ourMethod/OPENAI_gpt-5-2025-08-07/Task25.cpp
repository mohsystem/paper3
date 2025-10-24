#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> number_lines(const std::vector<std::string>& lines) {
    std::vector<std::string> result;
    result.reserve(lines.size());
    for (size_t i = 0; i < lines.size(); ++i) {
        result.push_back(std::to_string(i + 1) + ": " + lines[i]);
    }
    return result;
}

static void print_vector(const std::vector<std::string>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << "\"" << v[i] << "\"";
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    // Test 1: Empty input
    {
        std::vector<std::string> input{};
        auto output = number_lines(input);
        print_vector(output);
    }

    // Test 2: Basic example
    {
        std::vector<std::string> input{"a", "b", "c"};
        auto output = number_lines(input);
        print_vector(output);
    }

    // Test 3: Contains empty strings
    {
        std::vector<std::string> input{"", "line", ""};
        auto output = number_lines(input);
        print_vector(output);
    }

    // Test 4: Mixed content with spaces and punctuation
    {
        std::vector<std::string> input{"Hello, world!", "  leading space", "trailing space  ", "colon: test"};
        auto output = number_lines(input);
        print_vector(output);
    }

    // Test 5: Long string
    {
        std::string longStr(2000, 'x');
        std::vector<std::string> input{longStr, "short", longStr + " end"};
        auto output = number_lines(input);
        std::cout << "[";
        for (size_t i = 0; i < output.size(); ++i) {
            // Print only the length to keep console reasonable
            std::cout << "\"len=" << output[i].size() << "\"";
            if (i + 1 < output.size()) std::cout << ", ";
        }
        std::cout << "]\n";
    }

    return 0;
}