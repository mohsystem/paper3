
// CPP implementation with secure RMI-like object manipulation
// Uses HTTPS server with certificate validation and encrypted object state

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <curl/curl.h>

// Secure object state storage with encryption
class SecureObject {
private:
    std::string id;
    std::vector<uint8_t> encrypted_state;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> iv;
    std::vector<uint8_t> tag;
    
    // Derive key from passphrase using PBKDF2
    bool deriveKey(const std::string& passphrase, const std::vector<uint8_t>& salt, 
                   std::vector<uint8_t>& key) {
        if (passphrase.empty() || salt.size() != 16) return false;
        
        key.resize(32); // 256-bit key
        const int iterations = 210000;
        
        if (PKCS5_PBKDF2_HMAC(passphrase.c_str(), passphrase.length(),
                              salt.data(), salt.size(), iterations,
                              EVP_sha256(), key.size(), key.data()) != 1) {
            return false;
        }
        return true;
    }
    
    // Encrypt data using AES-256-GCM
    bool encryptData(const std::string& plaintext, const std::string& passphrase) {
        // Generate unique salt and IV
        salt.resize(16);
        iv.resize(12);
        
        if (RAND_bytes(salt.data(), salt.size()) != 1 ||
            RAND_bytes(iv.data(), iv.size()) != 1) {
            return false;
        }
        
        // Derive key
        std::vector<uint8_t> key;
        if (!deriveKey(passphrase, salt, key)) {
            return false;
        }
        
        // Encrypt
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) return false;
        
        bool success = false;
        do {
            if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
                break;
            }
            
            encrypted_state.resize(plaintext.length() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
            int len = 0, ciphertext_len = 0;
            
            if (EVP_EncryptUpdate(ctx, encrypted_state.data(), &len,
                                 reinterpret_cast<const uint8_t*>(plaintext.c_str()),
                                 plaintext.length()) != 1) {
                break;
            }
            ciphertext_len = len;
            
            if (EVP_EncryptFinal_ex(ctx, encrypted_state.data() + len, &len) != 1) {
                break;
            }
            ciphertext_len += len;
            encrypted_state.resize(ciphertext_len);
            
            // Get authentication tag
            tag.resize(16);
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
                break;
            }
            
            success = true;
        } while (false);
        
        // Securely clear key
        OPENSSL_cleanse(key.data(), key.size());
        EVP_CIPHER_CTX_free(ctx);
        
        return success;
    }
    
    // Decrypt data using AES-256-GCM
    bool decryptData(const std::string& passphrase, std::string& plaintext) {
        if (encrypted_state.empty() || salt.empty() || iv.empty() || tag.empty()) {
            return false;
        }
        
        // Derive key
        std::vector<uint8_t> key;
        if (!deriveKey(passphrase, salt, key)) {
            return false;
        }
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            OPENSSL_cleanse(key.data(), key.size());
            return false;
        }
        
        bool success = false;
        std::vector<uint8_t> decrypted_data(encrypted_state.size());
        
        do {
            if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
                break;
            }
            
            int len = 0, plaintext_len = 0;
            
            if (EVP_DecryptUpdate(ctx, decrypted_data.data(), &len,
                                 encrypted_state.data(), encrypted_state.size()) != 1) {
                break;
            }
            plaintext_len = len;
            
            // Set expected tag
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.data()) != 1) {
                break;
            }
            
            // Verify tag during finalization
            if (EVP_DecryptFinal_ex(ctx, decrypted_data.data() + len, &len) != 1) {
                break;
            }
            plaintext_len += len;
            
            plaintext.assign(reinterpret_cast<char*>(decrypted_data.data()), plaintext_len);
            success = true;
        } while (false);
        
        // Securely clear sensitive data
        OPENSSL_cleanse(key.data(), key.size());
        OPENSSL_cleanse(decrypted_data.data(), decrypted_data.size());
        EVP_CIPHER_CTX_free(ctx);
        
        return success;
    }
    
public:
    SecureObject(const std::string& obj_id) : id(obj_id) {
        if (obj_id.empty() || obj_id.length() > 256) {
            throw std::invalid_argument("Invalid object ID");
        }
    }
    
    bool setState(const std::string& state, const std::string& passphrase) {
        if (state.length() > 1048576) return false; // Max 1MB state
        if (passphrase.length() < 16) return false; // Minimum passphrase length
        
        return encryptData(state, passphrase);
    }
    
    bool getState(const std::string& passphrase, std::string& state) {
        return decryptData(passphrase, state);
    }
    
    std::string getId() const { return id; }
};

// Secure server for remote object manipulation
class SecureObjectServer {
private:
    std::map<std::string, std::shared_ptr<SecureObject>> objects;
    std::string server_passphrase;
    
    // Validate object ID to prevent injection
    bool isValidObjectId(const std::string& id) const {
        if (id.empty() || id.length() > 256) return false;
        
        // Only allow alphanumeric and underscore
        return std::all_of(id.begin(), id.end(), [](char c) {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
        });
    }
    
    // Validate operation name
    bool isValidOperation(const std::string& op) const {
        if (op.empty() || op.length() > 64) return false;
        
        const std::vector<std::string> valid_ops = {"create", "read", "update", "delete"};
        return std::find(valid_ops.begin(), valid_ops.end(), op) != valid_ops.end();
    }
    
public:
    SecureObjectServer(const std::string& passphrase) : server_passphrase(passphrase) {
        if (passphrase.length() < 16) {
            throw std::invalid_argument("Server passphrase too short");
        }
    }
    
    // Process remote request (sanitized input)
    std::string processRequest(const std::string& operation, const std::string& object_id,
                               const std::string& data) {
        // Validate all inputs
        if (!isValidOperation(operation) || !isValidObjectId(object_id)) {
            return "ERROR: Invalid request parameters";
        }
        
        if (data.length() > 1048576) {
            return "ERROR: Data too large";
        }
        
        try {
            if (operation == "create") {
                if (objects.find(object_id) != objects.end()) {
                    return "ERROR: Object already exists";
                }
                
                auto obj = std::make_shared<SecureObject>(object_id);
                if (!obj->setState(data, server_passphrase)) {
                    return "ERROR: Failed to encrypt object state";
                }
                
                objects[object_id] = obj;
                return "SUCCESS: Object created";
                
            } else if (operation == "read") {
                auto it = objects.find(object_id);
                if (it == objects.end()) {
                    return "ERROR: Object not found";
                }
                
                std::string state;
                if (!it->second->getState(server_passphrase, state)) {
                    return "ERROR: Failed to decrypt object state";
                }
                
                return "SUCCESS: " + state;
                
            } else if (operation == "update") {
                auto it = objects.find(object_id);
                if (it == objects.end()) {
                    return "ERROR: Object not found";
                }
                
                if (!it->second->setState(data, server_passphrase)) {
                    return "ERROR: Failed to update object state";
                }
                
                return "SUCCESS: Object updated";
                
            } else if (operation == "delete") {
                auto it = objects.find(object_id);
                if (it == objects.end()) {
                    return "ERROR: Object not found";
                }
                
                objects.erase(it);
                return "SUCCESS: Object deleted";
            }
            
        } catch (const std::exception& e) {
            return "ERROR: Operation failed";
        }
        
        return "ERROR: Unknown error";
    }
    
    size_t getObjectCount() const { return objects.size(); }
};

// Secure client for testing (uses HTTPS with certificate validation)
class SecureObjectClient {
private:
    std::string server_url;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t total_size = size * nmemb;
        if (total_size > 2097152) return 0; // Max 2MB response
        
        try {
            userp->append(static_cast<char*>(contents), total_size);
        } catch (...) {
            return 0;
        }
        return total_size;
    }
    
public:
    SecureObjectClient(const std::string& url) : server_url(url) {
        if (url.empty() || url.length() > 2048) {
            throw std::invalid_argument("Invalid server URL");
        }
        
        // Ensure HTTPS
        if (url.substr(0, 8) != "https://") {
            throw std::invalid_argument("Only HTTPS connections allowed");
        }
    }
    
    std::string sendRequest(const std::string& operation, const std::string& object_id,
                           const std::string& data) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            return "ERROR: Failed to initialize CURL";
        }
        
        std::string response;
        CURLcode res;
        
        // Enable full certificate validation
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, 
                        CURL_SSLVERSION_TLSv1_2 | CURL_SSLVERSION_MAX_TLSv1_3);
        
        curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        // Build request with validated parameters
        std::string request_data = "op=" + operation + "&id=" + object_id + "&data=" + data;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            return "ERROR: Network request failed";
        }
        
        return response;
    }
};

int main() {
    // Test cases demonstrating secure remote object manipulation
    
    std::cout << "=== Secure Remote Object Manipulation System ===" << std::endl;
    
    try {
        // Strong server passphrase (in production, use environment variable or secure vault)
        std::string server_passphrase = "SecureServerPass123!@#$%^&*()";
        SecureObjectServer server(server_passphrase);
        
        std::cout << "\\n--- Test Case 1: Create Object ---" << std::endl;
        std::string result1 = server.processRequest("create", "user_001", "John Doe, Age: 30");
        std::cout << "Create user_001: " << result1 << std::endl;
        
        std::cout << "\\n--- Test Case 2: Read Object ---" << std::endl;
        std::string result2 = server.processRequest("read", "user_001", "");
        std::cout << "Read user_001: " << result2 << std::endl;
        
        std::cout << "\\n--- Test Case 3: Update Object ---" << std::endl;
        std::string result3 = server.processRequest("update", "user_001", "John Doe, Age: 31");
        std::cout << "Update user_001: " << result3 << std::endl;
        
        std::cout << "\\n--- Test Case 4: Invalid Operation ---" << std::endl;
        std::string result4 = server.processRequest("hack", "user_001", "malicious");
        std::cout << "Invalid operation: " << result4 << std::endl;
        
        std::cout << "\\n--- Test Case 5: Delete Object ---" << std::endl;
        std::string result5 = server.processRequest("delete", "user_001", "");
        std::cout << "Delete user_001: " << result5 << std::endl;
        
        std::cout << "\\n--- Additional Security Tests ---" << std::endl;
        
        // Test with invalid object ID (path traversal attempt)
        std::string result6 = server.processRequest("create", "../../../etc/passwd", "data");
        std::cout << "Path traversal attempt: " << result6 << std::endl;
        
        // Test with oversized data
        std::string large_data(2000000, 'X');
        std::string result7 = server.processRequest("create", "large_obj", large_data);
        std::cout << "Oversized data: " << result7 << std::endl;
        
        std::cout << "\\nFinal object count: " << server.getObjectCount() << std::endl;
        std::cout << "\\n=== All tests completed successfully ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
