
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <regex>
#include <cstdio>

using namespace std;

bool validateCommand(const string& command) {
    // Basic validation - reject potentially dangerous commands
    vector<string> dangerousPatterns = {
        "rm\\\\s+-rf\\\\s+/",
        ":\\\\(\\\\)\\\\{.*:\\\\|:&\\\\}",  // fork bomb
        "dd\\\\s+if=",
        "mkfs",
        "format",
        ">\\\\s*/dev/sda"
    };
    
    for (const auto& pattern : dangerousPatterns) {
        regex re(pattern);
        if (regex_search(command, re)) {
            return false;
        }
    }
    
    return true;
}

string executeCommand(const string& command) {
    if (command.empty() || command.find_first_not_of(' ') == string::npos) {
        return "Error: Command cannot be empty";
    }
    
    if (!validateCommand(command)) {
        return "Error: Invalid or potentially dangerous command";
    }
    
    string result;
    array<char, 128> buffer;
    
    string fullCommand = "bash -c \\"" + command + "\\" 2>&1";
    
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCommand.c_str(), "r"), pclose);
    
    if (!pipe) {
        return "Error: Failed to execute command";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    // Remove trailing newline
    if (!result.empty() && result.back() == '\\n') {
        result.pop_back();
    }
    
    return result;
}

int main() {
    cout << "Test Case 1: echo 'Hello World'" << endl;
    cout << executeCommand("echo 'Hello World'") << endl;
    cout << endl;
    
    cout << "Test Case 2: ls -la | head -5" << endl;
    cout << executeCommand("ls -la | head -5") << endl;
    cout << endl;
    
    cout << "Test Case 3: date" << endl;
    cout << executeCommand("date") << endl;
    cout << endl;
    
    cout << "Test Case 4: pwd" << endl;
    cout << executeCommand("pwd") << endl;
    cout << endl;
    
    cout << "Test Case 5: echo $((5 + 3))" << endl;
    cout << executeCommand("echo $((5 + 3))") << endl;
    cout << endl;
    
    return 0;
}
