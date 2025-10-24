#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class Task190 {
public:
    /**
     * Transposes the given content where rows are separated by newlines
     * and columns by spaces.
     *
     * @param content The string content to transpose.
     * @return The transposed string content.
     */
    static std::string transposeContent(const std::string& content) {
        if (content.empty()) {
            return "";
        }

        std::stringstream content_stream(content);
        std::string line;
        std::vector<std::vector<std::string>> rows;

        while (std::getline(content_stream, line)) {
            if (line.empty()) continue; // Skip empty lines
            
            std::stringstream line_stream(line);
            std::string word;
            std::vector<std::string> current_row;
            while (line_stream >> word) {
                current_row.push_back(word);
            }
            if (!current_row.empty()) {
                rows.push_back(current_row);
            }
        }

        if (rows.empty()) {
            return "";
        }

        // Assumes all rows have the same number of columns as the first one
        size_t numRows = rows.size();
        size_t numCols = rows[0].size();

        std::ostringstream transposed_stream;
        for (size_t j = 0; j < numCols; ++j) {
            for (size_t i = 0; i < numRows; ++i) {
                // This check adds robustness for potentially jagged inputs
                if (j < rows[i].size()) {
                    transposed_stream << rows[i][j];
                }
                if (i < numRows - 1) {
                    transposed_stream << " ";
                }
            }
            if (j < numCols - 1) {
                transposed_stream << "\n";
            }
        }

        return transposed_stream.str();
    }
};

int main() {
    std::vector<std::string> testCases = {
        "name age\nalice 21\nryan 30",
        "a b c\nd e f\ng h i",
        "one two three four",
        "apple\nbanana\ncherry",
        ""
    };

    for (int i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input:" << std::endl;
        std::cout << (testCases[i].empty() ? "<empty>" : testCases[i]) << std::endl;
        std::cout << "Output:" << std::endl;
        std::string result = Task190::transposeContent(testCases[i]);
        std::cout << (result.empty() ? "<empty>" : result) << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}