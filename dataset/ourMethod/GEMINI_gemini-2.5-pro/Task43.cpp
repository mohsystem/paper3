#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <iterator>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/err.h>

// Constants for session management and cryptography
const std::string MAGIC_HEADER = "ENC1";
const uint8_t VERSION = 1;
const int KEY_SIZE = 32; // AES-256
const int SALT_SIZE = 16;
const int IV_SIZE = 12; // Recommended for GCM
const int TAG_SIZE = 16; // GCM authentication tag
const int PBKDF2_ITERATIONS = 210000;
const int SESSION_ID_SIZE = 32;
const std::string SESSION_DIR = "sessions";
const int SESSION_LIFETIME_SECONDS = 3600; // 1 hour

// Helper to print OpenSSL errors and throw an exception
void handle_openssl_errors() {
    unsigned long err_code;
    std::string err_details;
    while ((err_code = ERR_get_error())) {
        char err_msg[256];
        ERR_error_string_n(err_code, err_msg, sizeof(err_msg));
        err_details += std::string(err_msg) + "\n";
    }
    throw std::runtime_error("An OpenSSL error occurred: " + err_details);
}

// Generate cryptographically secure random bytes
std::vector<unsigned char> generate_secure_random_bytes(int size) {
    std::vector<unsigned char> buffer(size);
    if (RAND_bytes(buffer.data(), size) != 1) {
        handle_openssl_errors();
    }
    return buffer;
}

// Convert a vector of bytes to a hexadecimal string
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Derive a key from a password/master key using PBKDF2
std::vector<unsigned char> derive_key_from_password(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                           salt.data(), salt.size(),
                           PBKDF2_ITERATIONS, EVP_sha256(),
                           key.size(), key.data()) != 1) {
        handle_openssl_errors();
    }
    return key;
}

// Calculate the SHA-256 hash of a string and return it as a hex string
std::string sha256_hex(const std::string& input) {
    std::vector<unsigned char> hash(SHA256_DIGEST_LENGTH);
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) handle_openssl_errors();
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> mdctx_ptr(mdctx, EVP_MD_CTX_free);

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) handle_openssl_errors();
    if (EVP_DigestUpdate(mdctx, input.c_str(), input.length()) != 1) handle_openssl_errors();
    if (EVP_DigestFinal_ex(mdctx, hash.data(), NULL) != 1) handle_openssl_errors();

    return bytes_to_hex(hash);
}

// Represents the data stored in a session
struct SessionData {
    std::string user_id;
    std::chrono::system_clock::time_point expires_at;
};

// Simple serialization: user_id (null-terminated) followed by expiration (time_t)
std::vector<unsigned char> serialize_session(const SessionData& data) {
    std::vector<unsigned char> buffer;
    buffer.insert(buffer.end(), data.user_id.begin(), data.user_id.end());
    buffer.push_back('\0');

    time_t expiration_time = std::chrono::system_clock::to_time_t(data.expires_at);
    const unsigned char* time_bytes = reinterpret_cast<const unsigned char*>(&expiration_time);
    buffer.insert(buffer.end(), time_bytes, time_bytes + sizeof(time_t));
    return buffer;
}

// Deserializes session data from a byte vector
SessionData deserialize_session(const std::vector<unsigned char>& buffer) {
    SessionData data;
    auto null_terminator_pos = std::find(buffer.begin(), buffer.end(), '\0');
    if (null_terminator_pos == buffer.end() || std::distance(null_terminator_pos, buffer.end()) != (sizeof(time_t) + 1)) {
        throw std::runtime_error("Invalid session data format for deserialization.");
    }

    data.user_id = std::string(buffer.begin(), null_terminator_pos);
    
    time_t expiration_time;
    std::copy(null_terminator_pos + 1, buffer.end(), reinterpret_cast<unsigned char*>(&expiration_time));
    data.expires_at = std::chrono::system_clock::from_time_t(expiration_time);

    return data;
}

// Internal function to encrypt session data and store it in a file
bool encrypt_and_store_session(const std::string& session_id, const SessionData& data, const std::string& master_key) {
    auto plaintext = serialize_session(data);
    auto salt = generate_secure_random_bytes(SALT_SIZE);
    auto iv = generate_secure_random_bytes(IV_SIZE);
    auto key = derive_key_from_password(master_key, salt);

    std::vector<unsigned char> ciphertext(plaintext.size());
    std::vector<unsigned char> tag(TAG_SIZE);
    int len;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_ptr(ctx, EVP_CIPHER_CTX_free);

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1) handle_openssl_errors();
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) handle_openssl_errors();
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) handle_openssl_errors();
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) handle_openssl_errors();

    std::filesystem::create_directories(SESSION_DIR);
    std::string filename = SESSION_DIR + "/" + sha256_hex(session_id);
    std::ofstream out(filename, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    
    out.write(MAGIC_HEADER.c_str(), MAGIC_HEADER.size());
    out.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));
    out.write(reinterpret_cast<const char*>(salt.data()), salt.size());
    out.write(reinterpret_cast<const char*>(iv.data()), iv.size());
    out.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
    out.write(reinterpret_cast<const char*>(tag.data()), tag.size());

    return out.good();
}

// Internal function to retrieve and decrypt session data from a file
std::unique_ptr<SessionData> retrieve_and_decrypt_session(const std::string& session_id, const std::string& master_key) {
    std::string filename = SESSION_DIR + "/" + sha256_hex(session_id);
    std::ifstream in(filename, std::ios::binary | std::ios::ate);
    if (!in) return nullptr;

    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::vector<unsigned char> file_buffer(size);
    if (!in.read(reinterpret_cast<char*>(file_buffer.data()), size)) return nullptr;

    const size_t min_size = MAGIC_HEADER.size() + sizeof(VERSION) + SALT_SIZE + IV_SIZE + TAG_SIZE;
    if (size < min_size) return nullptr;

    size_t offset = 0;
    if (std::string(reinterpret_cast<const char*>(file_buffer.data()), MAGIC_HEADER.size()) != MAGIC_HEADER) return nullptr;
    offset += MAGIC_HEADER.size();

    if (file_buffer[offset] != VERSION) return nullptr;
    offset += sizeof(VERSION);

    std::vector<unsigned char> salt(file_buffer.begin() + offset, file_buffer.begin() + offset + SALT_SIZE);
    offset += SALT_SIZE;

    std::vector<unsigned char> iv(file_buffer.begin() + offset, file_buffer.begin() + offset + IV_SIZE);
    offset += IV_SIZE;

    size_t ciphertext_size = size - min_size;
    std::vector<unsigned char> ciphertext(file_buffer.begin() + offset, file_buffer.begin() + offset + ciphertext_size);
    offset += ciphertext_size;
    
    std::vector<unsigned char> tag(file_buffer.begin() + offset, file_buffer.begin() + offset + TAG_SIZE);

    auto key = derive_key_from_password(master_key, salt);
    std::vector<unsigned char> plaintext(ciphertext.size());
    int len;
    int plaintext_len;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return nullptr;
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_ptr(ctx, EVP_CIPHER_CTX_free);

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1) return nullptr;
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) return nullptr;
    plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data()) != 1) return nullptr;
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) return nullptr; // Tag verification happens here
    plaintext_len += len;
    plaintext.resize(plaintext_len);
    
    try {
        auto session = std::make_unique<SessionData>(deserialize_session(plaintext));
        if (session->expires_at < std::chrono::system_clock::now()) {
            return nullptr; // Expired session
        }
        return session;
    } catch (const std::runtime_error&) {
        return nullptr;
    }
}

// ---- Public API Functions ----

// Creates a new session for a user and returns a unique session ID.
std::string create_session(const std::string& user_id, const std::string& master_key) {
    auto session_id_bytes = generate_secure_random_bytes(SESSION_ID_SIZE);
    std::string session_id = bytes_to_hex(session_id_bytes);
    
    SessionData data;
    data.user_id = user_id;
    data.expires_at = std::chrono::system_clock::now() + std::chrono::seconds(SESSION_LIFETIME_SECONDS);

    if (encrypt_and_store_session(session_id, data, master_key)) {
        return session_id;
    } else {
        return "";
    }
}

// Retrieves session data for a given session ID. Returns nullptr if invalid, expired, or not found.
std::unique_ptr<SessionData> get_session(const std::string& session_id, const std::string& master_key) {
    if (session_id.empty()) return nullptr;
    return retrieve_and_decrypt_session(session_id, master_key);
}

// Deletes the session file, effectively terminating the session.
void terminate_session(const std::string& session_id) {
    if (session_id.empty()) return;
    std::string filename = SESSION_DIR + "/" + sha256_hex(session_id);
    std::error_code ec;
    std::filesystem::remove(filename, ec);
}

int main() {
    // Rule #1 & #2: Avoid hardcoding credentials. Use environment variables.
    const char* master_key_env = std::getenv("SESSION_MASTER_KEY");
    std::string master_key;
    if (master_key_env && std::strlen(master_key_env) > 0) {
        master_key = master_key_env;
    } else {
        std::cerr << "WARNING: SESSION_MASTER_KEY environment variable not set. Using a weak, hardcoded key for demonstration." << std::endl;
        master_key = "a-very-weak-default-master-key-for-dev";
    }

    std::cout << "--- Test Case 1: Create and retrieve a valid session ---" << std::endl;
    std::string user_id = "user123";
    std::string session_id = create_session(user_id, master_key);
    if (!session_id.empty()) {
        std::cout << "Session created successfully." << std::endl;
        auto session = get_session(session_id, master_key);
        if (session && session->user_id == user_id) {
            std::cout << "Session retrieved successfully for user: " << session->user_id << std::endl;
        } else {
            std::cerr << "TEST FAILED: Could not retrieve valid session." << std::endl;
        }
    } else {
        std::cerr << "TEST FAILED: Session creation failed." << std::endl;
    }
    
    std::cout << "\n--- Test Case 2: Attempt to retrieve a non-existent session ---" << std::endl;
    auto non_existent_session = get_session("nonexistent-session-id", master_key);
    if (!non_existent_session) {
        std::cout << "Correctly failed to retrieve non-existent session." << std::endl;
    } else {
        std::cerr << "TEST FAILED: Retrieved a non-existent session." << std::endl;
    }

    std::cout << "\n--- Test Case 3: Tamper with a session file (simulated) ---" << std::endl;
    std::string filename = SESSION_DIR + "/" + sha256_hex(session_id);
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (file) {
        file.seekp(-1, std::ios::end); // Go to the last byte (part of the tag)
        file.put('X'); // Tamper with the tag
        file.close();
        std::cout << "Session file tampered." << std::endl;
        auto tampered_session = get_session(session_id, master_key);
        if (!tampered_session) {
            std::cout << "Correctly failed to retrieve tampered session (decryption/tag check failed)." << std::endl;
            ERR_clear_error(); // Clear expected OpenSSL errors
        } else {
            std::cerr << "TEST FAILED: Retrieved a tampered session." << std::endl;
        }
    } else {
        std::cerr << "Could not open session file for tampering test." << std::endl;
    }

    std::cout << "\n--- Test Case 4: Terminate a session ---" << std::endl;
    terminate_session(session_id);
    std::cout << "Session termination requested." << std::endl;
    auto terminated_session = get_session(session_id, master_key);
    if (!terminated_session) {
        std::cout << "Correctly failed to retrieve terminated session." << std::endl;
    } else {
        std::cerr << "TEST FAILED: Retrieved a terminated session." << std::endl;
    }

    std::cout << "\n--- Test Case 5: Expired session ---" << std::endl;
    std::string expired_session_id_str = "expired-test-id";
    SessionData expired_data;
    expired_data.user_id = "expired_user";
    expired_data.expires_at = std::chrono::system_clock::now() - std::chrono::seconds(1);
    if(encrypt_and_store_session(expired_session_id_str, expired_data, master_key)){
        std::cout << "Expired session file created." << std::endl;
        auto retrieved_expired = get_session(expired_session_id_str, master_key);
        if(!retrieved_expired){
            std::cout << "Correctly failed to retrieve expired session." << std::endl;
        } else {
            std::cerr << "TEST FAILED: Retrieved an expired session." << std::endl;
        }
        terminate_session(expired_session_id_str);
    } else {
        std::cerr << "TEST FAILED: Could not create expired session for testing." << std::endl;
    }

    return 0;
}