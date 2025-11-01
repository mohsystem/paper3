#include <iostream>
#include <string>
#include <sstream>

/**
 * Reads a username from a request stream and writes a greeting to a response stream.
 * Assumes the request body is in the format "username=[some_name]".
 * @param request The input stream containing the client request.
 * @param response The output stream to write the response to.
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    std::string requestLine;
    std::getline(request, requestLine);

    std::string username = "guest"; // Default username

    std::string prefix = "username=";
    // Check if the request line starts with the correct prefix
    if (requestLine.rfind(prefix, 0) == 0) {
        username = requestLine.substr(prefix.length());
    }

    response << "hello " << username;
    response.flush();
}

// Helper function for running a test case
void test(const std::string& requestBody, const std::string& expectedResponse) {
    // Simulate request and response streams using std::stringstream
    std::stringstream request(requestBody);
    std::stringstream response;
    
    // Call the function
    http_hello_page(request, response);
    
    // Get the actual response from the stringstream
    std::string actualResponse = response.str();
    
    // Print results
    std::cout << "Request: \"" << requestBody << "\"" << std::endl;
    std::cout << "Expected: \"" << expectedResponse << "\"" << std::endl;
    std::cout << "Actual  : \"" << actualResponse << "\"" << std::endl;
    std::cout << "Result  : " << (expectedResponse == actualResponse ? "PASS" : "FAIL") << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Standard username
    test("username=Alice", "hello Alice");

    // Test Case 2: Username with spaces (no trim in this version)
    test("username=  Bob  ", "hello   Bob  ");
    
    // Test Case 3: Empty username
    test("username=", "hello ");
    
    // Test Case 4: Malformed request (doesn't start with "username=")
    test("user=Charlie", "hello guest");
    
    // Test Case 5: Empty request
    test("", "hello guest");
    
    return 0;
}