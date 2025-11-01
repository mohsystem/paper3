#include <iostream>
#include <string>
#include <vector>

/**
 * Simulates handling a request for the /hello page.
 * @param username The request body, expected to be a username.
 * @return The constructed HTML response body.
 */
std::string http_hello_page(const std::string& username) {
    // The server constructs the response body (the HTML page) into the outputbuf variable.
    std::string outputbuf = "<html><body><h1>Hello, " + username + "!</h1></body></html>";
    
    // In a real server, we would write this buffer to the response stream.
    // Here, we return it as a string to simulate the process.
    // The "closing" of the stream is handled by the function returning and RAII.
    return outputbuf;
}

int main() {
    // 5 test cases
    std::vector<std::string> testUsernames = {"Alice", "Bob", "Charlie", "David", "Eve"};

    for (int i = 0; i < testUsernames.size(); ++i) {
        // The server reads the request body (the username) into the username variable.
        std::string username = testUsernames[i];

        std::cout << "--- Test Case " << i + 1 << " ---" << std::endl;
        std::cout << "Received username: " << username << std::endl;

        // The function http_hello_page is called when the server receives a request for the URL /hello.
        std::string response = http_hello_page(username);

        // The server writes the response body to the response stream.
        // (We print it to the console to simulate this)
        std::cout << "Generated response: " << response << std::endl << std::endl;
    }
    // The server is now ready to handle the next request (simulated by the loop).

    return 0;
}