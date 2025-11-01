#include <iostream>
#include <string>
#include <vector>

class Lse125 {
public:
    /**
     * @brief Trims trailing whitespace from a std::string.
     * 
     * @param s The string to trim.
     * @return A new string with trailing whitespace removed.
     */
    static std::string trimTrailingWhitespace(std::string s) {
        // Define whitespace characters.
        const std::string WHITESPACE = " \t\n\r\f\v";
        // Find the last character that is not a whitespace.
        size_t endpos = s.find_last_not_of(WHITESPACE);
        
        if (std::string::npos != endpos) {
            // Erase all characters after the last non-whitespace character.
            s.erase(endpos + 1);
        } else {
            // If no non-whitespace character is found, the string is all whitespace.
            s.clear();
        }
        return s;
    }

    /**
     * @brief Main logic that processes arguments or runs test cases.
     * 
     * @param argc Argument count.
     * @param argv Argument values.
     */
    static void run(int argc, char* argv[]) {
        if (argc > 1) {
            std::cout << "Processing command-line arguments:" << std::endl;
            // argv[0] is the program name, start from index 1.
            for (int i = 1; i < argc; ++i) {
                // Safely handle C-style string by converting to std::string.
                std::string arg(argv[i]);
                std::string trimmedArg = trimTrailingWhitespace(arg);
                std::cout << "Original: \"" << arg << "\" -> Trimmed: \"" << trimmedArg << "\"" << std::endl;
            }
        } else {
            std::cout << "No command-line arguments provided. Running test cases:" << std::endl;
            std::vector<std::string> testCases = {
                "Hello World   ",
                "  leading and trailing  \t\n",
                "NoWhitespace",
                "   ",
                ""
            };
            for (const auto& test : testCases) {
                std::string trimmedTest = trimTrailingWhitespace(test);
                std::cout << "Original: \"" << test << "\" -> Trimmed: \"" << trimmedTest << "\"" << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    Lse125::run(argc, argv);
    return 0;
}