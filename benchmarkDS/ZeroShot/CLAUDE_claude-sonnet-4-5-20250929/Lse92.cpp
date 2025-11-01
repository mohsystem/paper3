
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <stdexcept>
#include <openssl/sha.h>
#include <openssl/rand.h>

using namespace std;

class Lse92 {
public:
    struct RegistrationResult {
        string username;
        string hashedPassword;
        string salt;
        
        RegistrationResult(const string& u, const string& h, const string& s)
            : username(u), hashedPassword(h), salt(s) {}
    };
    
    static string base64_encode(const unsigned char* data, size_t len) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        string result;
        int val = 0;
        int valb = -6;
        
        for (size_t i = 0; i < len; i++) {
            val = (val << 8) + data[i];
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        return result;
    }
    
    static string base64_decode(const string& encoded) {
        static const string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        string result;
        int val = 0;
        int valb = -8;
        
        for (char c : encoded) {
            if (c == '=') break;
            size_t pos = base64_chars.find(c);
            if (pos == string::npos) continue;
            val = (val << 6) + pos;
            valb += 6;
            if (valb >= 0) {
                result.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }
    
    static string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        return base64_encode(salt, 16);
    }
    
    static string hashPassword(const string& password, const string& salt) {
        string decoded_salt = base64_decode(salt);
        string combined = decoded_salt + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static RegistrationResult registerUser(const string& username, const string& password) {
        if (username.empty() || username.find_first_not_of(' ') == string::npos) {
            throw invalid_argument("Username cannot be empty");
        }
        if (password.length() < 8) {
            throw invalid_argument("Password must be at least 8 characters");
        }
        
        string salt = generateSalt();
        string hashedPassword = hashPassword(password, salt);
        
        return RegistrationResult(username, hashedPassword, salt);
    }
};

int main() {
    // Test case 1
    try {
        auto result1 = Lse92::registerUser("john_doe", "SecurePass123");
        cout << "Test 1 - Username: " << result1.username << endl;
        cout << "Hashed Password: " << result1.hashedPassword << endl;
        cout << "Salt: " << result1.salt << endl << endl;
    } catch (const exception& e) {
        cout << "Test 1 failed: " << e.what() << endl;
    }
    
    // Test case 2
    try {
        auto result2 = Lse92::registerUser("alice_smith", "MyP@ssw0rd!");
        cout << "Test 2 - Username: " << result2.username << endl;
        cout << "Hashed Password: " << result2.hashedPassword << endl;
        cout << "Salt: " << result2.salt << endl << endl;
    } catch (const exception& e) {
        cout << "Test 2 failed: " << e.what() << endl;
    }
    
    // Test case 3 - Empty username
    try {
        auto result3 = Lse92::registerUser("", "Password123");
        cout << "Test 3 - Username: " << result3.username << endl;
    } catch (const exception& e) {
        cout << "Test 3 failed (expected): " << e.what() << endl << endl;
    }
    
    // Test case 4 - Short password
    try {
        auto result4 = Lse92::registerUser("bob_jones", "short");
        cout << "Test 4 - Username: " << result4.username << endl;
    } catch (const exception& e) {
        cout << "Test 4 failed (expected): " << e.what() << endl << endl;
    }
    
    // Test case 5
    try {
        auto result5 = Lse92::registerUser("emma_wilson", "StrongPassword2024");
        cout << "Test 5 - Username: " << result5.username << endl;
        cout << "Hashed Password: " << result5.hashedPassword << endl;
        cout << "Salt: " << result5.salt << endl;
    } catch (const exception& e) {
        cout << "Test 5 failed: " << e.what() << endl;
    }
    
    return 0;
}
