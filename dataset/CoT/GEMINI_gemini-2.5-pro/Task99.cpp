#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <cstdio> // For std::remove

class Task99 {
public:
    /**
     * Reads a file and returns a vector of lines that start with numbers and end with a dot.
     *
     * @param filePath The path to the file to read.
     * @return A vector of matching lines.
     */
    static std::vector<std::string> findMatchingLines(const std::string& filePath) {
        std::vector<std::string> matchingLines;
        std::ifstream inputFile(filePath);

        if (!inputFile.is_open()) {
            std::cerr << "Error: Could not open file " << filePath << std::endl;
            return matchingLines;
        }

        // Regex: ^\d+ -> starts with one or more digits
        //        .*   -> followed by any character, zero or more times
        //        \.   -> a literal dot
        //        $    -> end of the line
        // C++ requires backslashes to be escaped in string literals.
        std::regex pattern("^\\d+.*\\.$");
        
        std::string line;
        while (std::getline(inputFile, line)) {
            if (std::regex_match(line, pattern)) {
                matchingLines.push_back(line);
            }
        }
        
        // The file is automatically closed by the ifstream destructor (RAII)
        return matchingLines;
    }
};

int main() {
    const std::string testFileName = "test_Task99.txt";

    // Create a test file with some content
    {
        std::ofstream writer(testFileName);
        if (!writer.is_open()) {
            std::cerr << "Error creating test file." << std::endl;
            return 1;
        }
        writer << "1. This is the first test case.\n";
        writer << "This line does not start with a number.\n";
        writer << "2. Second test case.\n";
        writer << "3 A line without a dot at the end\n";
        writer << "4. Test case three is here.\n";
        writer << "Another non-matching line.\n";
        writer << "55. Test case four.\n";
        writer << "99999. Final test case.\n";
        writer << "The end.\n";
        writer << "6 is not a match either\n";
    } // writer is closed here by its destructor

    std::cout << "--- Reading from file: " << testFileName << " ---" << std::endl;
    std::vector<std::string> matched = Task99::findMatchingLines(testFileName);

    std::cout << "Found " << matched.size() << " matching lines:" << std::endl;
    for (const auto& line : matched) {
        std::cout << line << std::endl;
    }

    // Clean up the test file
    std::remove(testFileName.c_str());
    
    return 0;
}