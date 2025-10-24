#include <iostream>
#include <fstream>
#include <string>

/**
 * Creates an HTML file with the given content.
 *
 * @param content The user-supplied string to display on the web page.
 * @param filename The name of the HTML file to create.
 */
void createWebPage(const std::string& content, const std::string& filename) {
    std::ofstream outFile(filename);

    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open the file " << filename << " for writing." << std::endl;
        return;
    }

    outFile << "<!DOCTYPE html>\n"
            << "<html lang=\"en\">\n"
            << "<head>\n"
            << "    <meta charset=\"UTF-8\">\n"
            << "    <title>User Input Display</title>\n"
            << "</head>\n"
            << "<body>\n"
            << "    <h1>User-Supplied Content:</h1>\n"
            << "    <p>" << content << "</p>\n"
            << "</body>\n"
            << "</html>";

    outFile.close();
    std::cout << "Successfully created " << filename << std::endl;
}

int main() {
    // 5 Test Cases
    createWebPage("Hello, World!", "cpp_test1.html");
    createWebPage("This is a test from C++.", "cpp_test2.html");
    createWebPage("12345", "cpp_test3.html");
    createWebPage("Special characters: <script>alert('xss')</script>", "cpp_test4.html");
    createWebPage("A longer sentence to see how it wraps inside the paragraph tag on the web page.", "cpp_test5.html");
    
    return 0;
}