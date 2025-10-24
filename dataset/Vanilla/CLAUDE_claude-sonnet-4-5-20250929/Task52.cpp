
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

const int KEY = 123;

// Base64 encoding characters
static const string base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

string base64_encode(const vector<unsigned char>& data) {
    string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    int size = data.size();
    int pos = 0;

    while (size--) {
        char_array_3[i++] = data[pos++];
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(int j = i; j < 3; j++)
            char_array_3[j] = '\\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (int j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];

        while(i++ < 3)
            ret += '=';
    }

    return ret;
}

vector<unsigned char> base64_decode(const string& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in_] != '=')) {
        if (!isalnum(encoded_string[in_]) && encoded_string[in_] != '+' && encoded_string[in_] != '/') break;
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (int j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (int j = 0; j < i - 1; j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}

vector<unsigned char> encryptDecrypt(const vector<unsigned char>& data, int key) {
    vector<unsigned char> result;
    for (unsigned char byte : data) {
        result.push_back(byte ^ key);
    }
    return result;
}

void encryptFile(const string& inputFile, const string& outputFile, int key) {
    ifstream inFile(inputFile, ios::binary);
    vector<unsigned char> data((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();
    
    vector<unsigned char> encrypted = encryptDecrypt(data, key);
    
    ofstream outFile(outputFile, ios::binary);
    outFile.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
    outFile.close();
}

void decryptFile(const string& inputFile, const string& outputFile, int key) {
    ifstream inFile(inputFile, ios::binary);
    vector<unsigned char> data((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();
    
    vector<unsigned char> decrypted = encryptDecrypt(data, key);
    
    ofstream outFile(outputFile, ios::binary);
    outFile.write(reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
    outFile.close();
}

string encryptString(const string& plaintext, int key) {
    vector<unsigned char> data(plaintext.begin(), plaintext.end());
    vector<unsigned char> encrypted = encryptDecrypt(data, key);
    return base64_encode(encrypted);
}

string decryptString(const string& encrypted, int key) {
    vector<unsigned char> encryptedBytes = base64_decode(encrypted);
    vector<unsigned char> decrypted = encryptDecrypt(encryptedBytes, key);
    return string(decrypted.begin(), decrypted.end());
}

int main() {
    cout << "File Encryption/Decryption Program" << endl;
    cout << "===================================" << endl << endl;
    
    // Test Case 1: Simple String Encryption
    cout << "Test Case 1: Simple String Encryption" << endl;
    string text1 = "Hello World!";
    string encrypted1 = encryptString(text1, KEY);
    string decrypted1 = decryptString(encrypted1, KEY);
    cout << "Original: " << text1 << endl;
    cout << "Encrypted: " << encrypted1 << endl;
    cout << "Decrypted: " << decrypted1 << endl;
    cout << "Match: " << (text1 == decrypted1 ? "true" : "false") << endl << endl;
    
    // Test Case 2: Longer Text Encryption
    cout << "Test Case 2: Longer Text Encryption" << endl;
    string text2 = "This is a test message with numbers 12345 and symbols @#$%";
    string encrypted2 = encryptString(text2, KEY);
    string decrypted2 = decryptString(encrypted2, KEY);
    cout << "Original: " << text2 << endl;
    cout << "Encrypted: " << encrypted2 << endl;
    cout << "Decrypted: " << decrypted2 << endl;
    cout << "Match: " << (text2 == decrypted2 ? "true" : "false") << endl << endl;
    
    // Test Case 3: Different Key
    cout << "Test Case 3: Different Key" << endl;
    string text3 = "Secret Message";
    int customKey = 255;
    string encrypted3 = encryptString(text3, customKey);
    string decrypted3 = decryptString(encrypted3, customKey);
    cout << "Original: " << text3 << endl;
    cout << "Encrypted: " << encrypted3 << endl;
    cout << "Decrypted: " << decrypted3 << endl;
    cout << "Match: " << (text3 == decrypted3 ? "true" : "false") << endl << endl;
    
    // Test Case 4: Empty String
    cout << "Test Case 4: Empty String" << endl;
    string text4 = "";
    string encrypted4 = encryptString(text4, KEY);
    string decrypted4 = decryptString(encrypted4, KEY);
    cout << "Original: '" << text4 << "'" << endl;
    cout << "Encrypted: '" << encrypted4 << "'" << endl;
    cout << "Decrypted: '" << decrypted4 << "'" << endl;
    cout << "Match: " << (text4 == decrypted4 ? "true" : "false") << endl << endl;
    
    // Test Case 5: File Encryption/Decryption
    cout << "Test Case 5: File Encryption/Decryption" << endl;
    try {
        string originalFile = "test_input.txt";
        string encryptedFile = "test_encrypted.bin";
        string decryptedFile = "test_decrypted.txt";
        
        // Create a test file
        ofstream ofs(originalFile);
        ofs << "This is a test file content.\\nLine 2 with more data.";
        ofs.close();
        
        // Encrypt file
        encryptFile(originalFile, encryptedFile, KEY);
        cout << "File encrypted: " << encryptedFile << endl;
        
        // Decrypt file
        decryptFile(encryptedFile, decryptedFile, KEY);
        cout << "File decrypted: " << decryptedFile << endl;
        
        // Verify content
        ifstream ifs(decryptedFile);
        stringstream buffer;
        buffer << ifs.rdbuf();
        cout << "Decrypted content: " << buffer.str() << endl;
        
    } catch (const exception& e) {
        cout << "File operation: " << e.what() << endl;
    }
    
    return 0;
}
