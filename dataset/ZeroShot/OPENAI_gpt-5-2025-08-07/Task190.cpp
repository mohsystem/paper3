#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

std::vector<std::string> transposeText(const std::vector<std::string>& lines) {
    if (lines.empty()) return {};
    std::vector<std::vector<std::string>> matrix;
    size_t cols = 0;

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string tok;
        while (iss >> tok) {
            tokens.push_back(tok);
        }
        if (i == 0) {
            cols = tokens.size();
            if (cols == 0) throw std::invalid_argument("First line has no tokens");
        } else {
            if (tokens.size() != cols) {
                throw std::invalid_argument("Inconsistent number of columns at row " + std::to_string(i));
            }
        }
        matrix.push_back(std::move(tokens));
    }

    std::vector<std::string> result(cols);
    for (size_t c = 0; c < cols; ++c) {
        std::string row;
        for (size_t r = 0; r < matrix.size(); ++r) {
            if (r > 0) row.push_back(' ');
            row.append(matrix[r][c]);
        }
        result[c] = std::move(row);
    }
    return result;
}

static void runTest(int idx, const std::vector<std::string>& input) {
    try {
        auto out = transposeText(input);
        std::cout << "Test " << idx << " output:" << std::endl;
        for (const auto& s : out) std::cout << s << std::endl;
    } catch (const std::exception& ex) {
        std::cout << "Test " << idx << " error: " << ex.what() << std::endl;
    }
    std::cout << "---" << std::endl;
}

int main() {
    // Test 1: Sample
    runTest(1, {
        "name age",
        "alice 21",
        "ryan 30"
    });
    // Test 2: Single row
    runTest(2, {
        "a b c"
    });
    // Test 3: Single column
    runTest(3, {
        "a",
        "b",
        "c"
    });
    // Test 4: Multiple spaces and trims
    runTest(4, {
        "x   y",
        " 1    2 "
    });
    // Test 5: 2x2
    runTest(5, {
        "p q",
        "r s"
    });
    return 0;
}