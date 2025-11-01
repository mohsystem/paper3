
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

using namespace std;

void httpHelloPage(istream& request, ostream& response) {
    string username = "";
    string line;
    
    // Read from request to find username parameter
    while (getline(request, line)) {
        size_t pos = line.find("username=");
        if (pos != string::npos) {
            size_t startIndex = pos + 9;
            size_t endIndex = line.find("&", startIndex);
            if (endIndex == string::npos) {
                endIndex = line.find(" ", startIndex);
            }
            if (endIndex == string::npos) {
                endIndex = line.length();
            }
            username = line.substr(startIndex, endIndex - startIndex);
            break;
        }
    }
    
    // Construct response
    string responseMessage = "Hello " + username;
    response << "HTTP/1.1 200 OK\\n";
    response << "Content-Type: text/html\\n";
    response << "\\n";
    response << responseMessage << "\\n";
}

int main() {
    // Test case 1
    istringstream request1("GET /hello?username=John HTTP/1.1\\n");
    ostringstream response1;
    httpHelloPage(request1, response1);
    cout << "Test 1:\\n" << response1.str() << "\\n" << endl;
    
    // Test case 2
    istringstream request2("POST /hello\\nusername=Alice");
    ostringstream response2;
    httpHelloPage(request2, response2);
    cout << "Test 2:\\n" << response2.str() << "\\n" << endl;
    
    // Test case 3
    istringstream request3("GET /hello?username=Bob&age=25 HTTP/1.1\\n");
    ostringstream response3;
    httpHelloPage(request3, response3);
    cout << "Test 3:\\n" << response3.str() << "\\n" << endl;
    
    // Test case 4
    istringstream request4("GET /hello?username=Charlie HTTP/1.1\\n");
    ostringstream response4;
    httpHelloPage(request4, response4);
    cout << "Test 4:\\n" << response4.str() << "\\n" << endl;
    
    // Test case 5
    istringstream request5("POST /hello\\nusername=Eve&password=secret");
    ostringstream response5;
    httpHelloPage(request5, response5);
    cout << "Test 5:\\n" << response5.str() << "\\n" << endl;
    
    return 0;
}
