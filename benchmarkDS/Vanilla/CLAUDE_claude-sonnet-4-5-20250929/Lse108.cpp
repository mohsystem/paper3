
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <cstring>

using namespace std;

string getentPasswd(const string& username) {
    if (username.empty()) {
        return "Error: Username cannot be empty";
    }
    
    // Construct the command
    string command = "getent passwd " + username;
    
    // Open pipe to execute command
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "Error: Failed to execute command";
    }
    
    // Read the output
    char buffer[256];
    string result = "";
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    // Close the pipe
    int status = pclose(pipe);
    
    // Remove trailing newline if present
    if (!result.empty() && result[result.length() - 1] == '\\n') {
        result.erase(result.length() - 1);
    }
    
    if (result.empty()) {
        return "Error: User not found or command failed";
    }
    
    return result;
}

int main() {
    // Test cases
    string testUsers[] = {"root", "nobody", "daemon", "bin", "sys"};
    
    cout << "Testing getent passwd command:\\n\\n";
    
    for (int i = 0; i < 5; i++) {
        cout << "User: " << testUsers[i] << endl;
        cout << getentPasswd(testUsers[i]) << endl;
        cout << "---" << endl;
    }
    
    return 0;
}
