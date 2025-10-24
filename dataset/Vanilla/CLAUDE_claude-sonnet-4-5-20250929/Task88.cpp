
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

string generateRandomString(int length) {
    string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distribution(0, chars.length() - 1);
    
    string result;
    for (int i = 0; i < length; i++) {
        result += chars[distribution(generator)];
    }
    
    return result;
}

string convertToUnicode(const string& input) {
    stringstream unicode;
    
    for (char c : input) {
        unicode << "\\\\u" << setfill('0') << setw(4) << hex << (int)(unsigned char)c;
    }
    
    return unicode.str();
}

string createTempFileWithUnicode(int stringLength) {
    // Create a random string
    string randomString = generateRandomString(stringLength);
    
    // Convert string to Unicode
    string unicodeString = convertToUnicode(randomString);
    
    // Create temporary file path
    string tempPath;
    
#ifdef _WIN32
    char tempPathBuf[MAX_PATH];
    char tempFileName[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPathBuf);
    GetTempFileNameA(tempPathBuf, "temp_", 0, tempFileName);
    tempPath = tempFileName;
#else
    char tempTemplate[] = "/tmp/temp_XXXXXX.txt";
    int fd = mkstemp(tempTemplate);
    if (fd != -1) {
        close(fd);
        tempPath = tempTemplate;
    }
#endif
    
    // Write Unicode string to file using UTF-8 encoding
    ofstream outFile(tempPath, ios::out | ios::binary);
    outFile << unicodeString;
    outFile.close();
    
    return tempPath;
}

int main() {
    // Test case 1: Create temp file with 10 character string
    cout << "Test Case 1:" << endl;
    string path1 = createTempFileWithUnicode(10);
    cout << "Temp file created at: " << path1 << endl;
    ifstream file1(path1);
    string content1((istreambuf_iterator<char>(file1)), istreambuf_iterator<char>());
    cout << "Content: " << content1 << endl << endl;
    file1.close();
    
    // Test case 2: Create temp file with 20 character string
    cout << "Test Case 2:" << endl;
    string path2 = createTempFileWithUnicode(20);
    cout << "Temp file created at: " << path2 << endl;
    ifstream file2(path2);
    string content2((istreambuf_iterator<char>(file2)), istreambuf_iterator<char>());
    cout << "Content: " << content2 << endl << endl;
    file2.close();
    
    // Test case 3: Create temp file with 5 character string
    cout << "Test Case 3:" << endl;
    string path3 = createTempFileWithUnicode(5);
    cout << "Temp file created at: " << path3 << endl;
    ifstream file3(path3);
    string content3((istreambuf_iterator<char>(file3)), istreambuf_iterator<char>());
    cout << "Content: " << content3 << endl << endl;
    file3.close();
    
    // Test case 4: Create temp file with 15 character string
    cout << "Test Case 4:" << endl;
    string path4 = createTempFileWithUnicode(15);
    cout << "Temp file created at: " << path4 << endl;
    ifstream file4(path4);
    string content4((istreambuf_iterator<char>(file4)), istreambuf_iterator<char>());
    cout << "Content: " << content4 << endl << endl;
    file4.close();
    
    // Test case 5: Create temp file with 25 character string
    cout << "Test Case 5:" << endl;
    string path5 = createTempFileWithUnicode(25);
    cout << "Temp file created at: " << path5 << endl;
    ifstream file5(path5);
    string content5((istreambuf_iterator<char>(file5)), istreambuf_iterator<char>());
    cout << "Content: " << content5 << endl << endl;
    file5.close();
    
    return 0;
}
