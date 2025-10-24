
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

// Magic bytes and version for file format validation
const char MAGIC[] = "ENC1";
const uint8_t VERSION = 1;
const size_t SALT_SIZE = 16;
const size_t IV_SIZE = 12;
const size_t TAG_SIZE = 16;
const size_t MAGIC_SIZE = 4;
const int PBKDF2_ITERATIONS = 210000;

// RAII wrapper for OpenSSL cipher context
class CipherContext {
    EVP_CIPHER_CTX* ctx;
public:
    CipherContext() : ctx(EVP_CIPHER_CTX_new()) {}
    ~CipherContext() { 
        if (ctx) {
            EVP_CIPHER_CTX_free(ctx);
        }
    }
    EVP_CIPHER_CTX* get() { return ctx; }
    CipherContext(const CipherContext&) = delete;
    CipherContext& operator=(const CipherContext&) = delete;
};

// Securely clear memory containing sensitive data
void secure_clear(void* ptr, size_t size) {
    if (ptr) {
        volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
        while (size--) {
            *p++ = 0;
        }
    }
}

// RAII wrapper for secure buffer that clears on destruction
class SecureBuffer {
    std::vector<unsigned char> data;
public:
    explicit SecureBuffer(size_t size) : data(size, 0) {}
    ~SecureBuffer() {
        secure_clear(data.data(), data.size());
    }
    unsigned char* get() { return data.data(); }
    size_t size() const { return data.size(); }
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
};

// Derive encryption key from passphrase using PBKDF2-HMAC-SHA-256
bool derive_key(const std::string& passphrase, const unsigned char* salt, 
                unsigned char* key, size_t key_len) {
    // Input validation
    if (passphrase.empty() || !salt || !key || key_len != 32) {
        return false;
    }
    
    // Use PBKDF2 with SHA-256 and high iteration count
    int result = PKCS5_PBKDF2_HMAC(
        passphrase.c_str(), passphrase.length(),
        salt, SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        key_len, key
    );
    
    return result == 1;
}

// Normalize and validate path to prevent directory traversal
bool validate_path(const std::string& base_dir, const std::string& user_path, 
                   std::string& resolved_path) {
    // Validate inputs are not empty
    if (base_dir.empty() || user_path.empty()) {
        return false;
    }
    
    // Resolve base directory to absolute path
    char base_real[PATH_MAX];
    if (!realpath(base_dir.c_str(), base_real)) {
        return false;
    }
    
    // Construct full path
    std::string full_path = std::string(base_real) + "/" + user_path;
    
    // Resolve to absolute path
    char resolved[PATH_MAX];
    if (!realpath(full_path.c_str(), resolved)) {
        // Path might not exist yet for new files, check parent
        size_t last_slash = full_path.find_last_of('/');
        if (last_slash == std::string::npos) {
            return false;
        }
        std::string parent = full_path.substr(0, last_slash);
        char parent_real[PATH_MAX];
        if (!realpath(parent.c_str(), parent_real)) {
            return false;
        }
        std::string filename = full_path.substr(last_slash + 1);
        resolved_path = std::string(parent_real) + "/" + filename;
    } else {
        resolved_path = resolved;
    }
    
    // Ensure resolved path is within base directory
    size_t base_len = strlen(base_real);
    if (resolved_path.length() < base_len || 
        resolved_path.compare(0, base_len, base_real) != 0) {
        return false;
    }
    
    // Reject symlinks - get stat without following links
    struct stat st;
    if (lstat(resolved_path.c_str(), &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            return false;
        }
    }
    
    return true;
}

// Atomic file write: write to temp then rename
bool atomic_write(const std::string& filepath, const std::vector<unsigned char>& data) {
    // Create temporary file in same directory
    std::string temp_path = filepath + ".tmp";
    
    // Open with exclusive create mode
    std::ofstream ofs(temp_path, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        return false;
    }
    
    // Set restrictive permissions (owner read/write only)
    chmod(temp_path.c_str(), S_IRUSR | S_IWUSR);
    
    // Write data
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!ofs) {
        ofs.close();
        unlink(temp_path.c_str());
        return false;
    }
    
    // Flush to disk
    ofs.flush();
    if (!ofs) {
        ofs.close();
        unlink(temp_path.c_str());
        return false;
    }
    
    ofs.close();
    
    // Sync to ensure data is on disk
    int fd = open(temp_path.c_str(), O_RDONLY);
    if (fd < 0) {
        unlink(temp_path.c_str());
        return false;
    }
    fsync(fd);
    close(fd);
    
    // Atomic rename
    if (rename(temp_path.c_str(), filepath.c_str()) != 0) {
        unlink(temp_path.c_str());
        return false;
    }
    
    return true;
}

// Encrypt file using AES-256-GCM
bool encrypt_file(const std::string& base_dir, const std::string& input_file, 
                  const std::string& output_file, const std::string& passphrase) {
    // Validate passphrase
    if (passphrase.empty() || passphrase.length() < 8) {
        std::cerr << "Error: Passphrase too short" << std::endl;
        return false;
    }
    
    // Validate and normalize paths
    std::string input_path, output_path;
    if (!validate_path(base_dir, input_file, input_path) ||
        !validate_path(base_dir, output_file, output_path)) {
        std::cerr << "Error: Invalid file path" << std::endl;
        return false;
    }
    
    // Read input file
    std::ifstream ifs(input_path, std::ios::binary);
    if (!ifs) {
        std::cerr << "Error: Cannot open input file" << std::endl;
        return false;
    }
    
    // Get file size and validate
    ifs.seekg(0, std::ios::end);
    std::streamsize file_size = ifs.tellg();
    if (file_size < 0 || file_size > 100 * 1024 * 1024) { // Limit to 100MB
        std::cerr << "Error: Invalid file size" << std::endl;
        return false;
    }
    ifs.seekg(0, std::ios::beg);
    
    std::vector<unsigned char> plaintext(file_size);
    ifs.read(reinterpret_cast<char*>(plaintext.data()), file_size);
    if (!ifs) {
        std::cerr << "Error: Cannot read input file" << std::endl;
        return false;
    }
    ifs.close();
    
    // Generate random salt using cryptographically secure RNG
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        std::cerr << "Error: Failed to generate salt" << std::endl;
        return false;
    }
    
    // Derive key from passphrase
    SecureBuffer key(32);
    if (!derive_key(passphrase, salt, key.get(), key.size())) {
        std::cerr << "Error: Key derivation failed" << std::endl;
        return false;
    }
    
    // Generate random IV using cryptographically secure RNG
    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        std::cerr << "Error: Failed to generate IV" << std::endl;
        return false;
    }
    
    // Initialize cipher context
    CipherContext ctx;
    if (!ctx.get()) {
        std::cerr << "Error: Failed to create cipher context" << std::endl;
        return false;
    }
    
    // Initialize encryption with AES-256-GCM
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, 
                           key.get(), iv) != 1) {
        std::cerr << "Error: Encryption initialization failed" << std::endl;
        return false;
    }
    
    // Allocate buffer for ciphertext (may be slightly larger than plaintext)
    std::vector<unsigned char> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int len = 0;
    int ciphertext_len = 0;
    
    // Encrypt plaintext
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, 
                          plaintext.data(), plaintext.size()) != 1) {
        std::cerr << "Error: Encryption failed" << std::endl;
        return false;
    }
    ciphertext_len = len;
    
    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) {
        std::cerr << "Error: Encryption finalization failed" << std::endl;
        return false;
    }
    ciphertext_len += len;
    
    // Get authentication tag
    unsigned char tag[TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        std::cerr << "Error: Failed to get authentication tag" << std::endl;
        return false;
    }
    
    // Build output: [magic][version][salt][iv][ciphertext][tag]
    std::vector<unsigned char> output;
    output.reserve(MAGIC_SIZE + 1 + SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE);
    
    output.insert(output.end(), MAGIC, MAGIC + MAGIC_SIZE);
    output.push_back(VERSION);
    output.insert(output.end(), salt, salt + SALT_SIZE);
    output.insert(output.end(), iv, iv + IV_SIZE);
    output.insert(output.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);
    output.insert(output.end(), tag, tag + TAG_SIZE);
    
    // Atomic write to output file
    if (!atomic_write(output_path, output)) {
        std::cerr << "Error: Failed to write output file" << std::endl;
        return false;
    }
    
    // Clear sensitive data
    secure_clear(plaintext.data(), plaintext.size());
    secure_clear(tag, TAG_SIZE);
    
    return true;
}

// Decrypt file using AES-256-GCM
bool decrypt_file(const std::string& base_dir, const std::string& input_file, 
                  const std::string& output_file, const std::string& passphrase) {
    // Validate passphrase
    if (passphrase.empty()) {
        std::cerr << "Error: Empty passphrase" << std::endl;
        return false;
    }
    
    // Validate and normalize paths
    std::string input_path, output_path;
    if (!validate_path(base_dir, input_file, input_path) ||
        !validate_path(base_dir, output_file, output_path)) {
        std::cerr << "Error: Invalid file path" << std::endl;
        return false;
    }
    
    // Read encrypted file
    std::ifstream ifs(input_path, std::ios::binary);
    if (!ifs) {
        std::cerr << "Error: Cannot open input file" << std::endl;
        return false;
    }
    
    // Get file size and validate minimum size
    ifs.seekg(0, std::ios::end);
    std::streamsize file_size = ifs.tellg();
    size_t min_size = MAGIC_SIZE + 1 + SALT_SIZE + IV_SIZE + TAG_SIZE;
    if (file_size < static_cast<std::streamsize>(min_size) || 
        file_size > 100 * 1024 * 1024) {
        std::cerr << "Error: Invalid file size" << std::endl;
        return false;
    }
    ifs.seekg(0, std::ios::beg);
    
    std::vector<unsigned char> encrypted_data(file_size);
    ifs.read(reinterpret_cast<char*>(encrypted_data.data()), file_size);
    if (!ifs) {
        std::cerr << "Error: Cannot read input file" << std::endl;
        return false;
    }
    ifs.close();
    
    // Parse and validate file format
    size_t offset = 0;
    
    // Validate magic bytes
    if (memcmp(encrypted_data.data() + offset, MAGIC, MAGIC_SIZE) != 0) {
        std::cerr << "Error: Invalid file format" << std::endl;
        return false;
    }
    offset += MAGIC_SIZE;
    
    // Validate version
    if (encrypted_data[offset] != VERSION) {
        std::cerr << "Error: Unsupported file version" << std::endl;
        return false;
    }
    offset += 1;
    
    // Extract salt
    unsigned char salt[SALT_SIZE];
    memcpy(salt, encrypted_data.data() + offset, SALT_SIZE);
    offset += SALT_SIZE;
    
    // Extract IV
    unsigned char iv[IV_SIZE];
    memcpy(iv, encrypted_data.data() + offset, IV_SIZE);
    offset += IV_SIZE;
    
    // Calculate ciphertext length
    size_t ciphertext_len = encrypted_data.size() - offset - TAG_SIZE;
    if (ciphertext_len == 0 || ciphertext_len > encrypted_data.size()) {
        std::cerr << "Error: Invalid ciphertext length" << std::endl;
        return false;
    }
    
    // Extract ciphertext
    std::vector<unsigned char> ciphertext(ciphertext_len);
    memcpy(ciphertext.data(), encrypted_data.data() + offset, ciphertext_len);
    offset += ciphertext_len;
    
    // Extract authentication tag
    unsigned char tag[TAG_SIZE];
    memcpy(tag, encrypted_data.data() + offset, TAG_SIZE);
    
    // Derive key from passphrase
    SecureBuffer key(32);
    if (!derive_key(passphrase, salt, key.get(), key.size())) {
        std::cerr << "Error: Key derivation failed" << std::endl;
        return false;
    }
    
    // Initialize cipher context
    CipherContext ctx;
    if (!ctx.get()) {
        std::cerr << "Error: Failed to create cipher context" << std::endl;
        return false;
    }
    
    // Initialize decryption with AES-256-GCM
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, 
                           key.get(), iv) != 1) {
        std::cerr << "Error: Decryption initialization failed" << std::endl;
        return false;
    }
    
    // Allocate buffer for plaintext
    std::vector<unsigned char> plaintext(ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int len = 0;
    int plaintext_len = 0;
    
    // Decrypt ciphertext
    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, 
                          ciphertext.data(), ciphertext_len) != 1) {
        std::cerr << "Error: Decryption failed" << std::endl;
        secure_clear(plaintext.data(), plaintext.size());
        return false;
    }
    plaintext_len = len;
    
    // Set expected authentication tag
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        std::cerr << "Error: Failed to set authentication tag" << std::endl;
        secure_clear(plaintext.data(), plaintext.size());
        return false;
    }
    
    // Finalize decryption and verify authentication tag
    if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1) {
        std::cerr << "Error: Authentication verification failed" << std::endl;
        secure_clear(plaintext.data(), plaintext.size());
        return false;
    }
    plaintext_len += len;
    
    // Atomic write to output file
    std::vector<unsigned char> output(plaintext.begin(), plaintext.begin() + plaintext_len);
    if (!atomic_write(output_path, output)) {
        std::cerr << "Error: Failed to write output file" << std::endl;
        secure_clear(plaintext.data(), plaintext.size());
        return false;
    }
    
    // Clear sensitive data
    secure_clear(plaintext.data(), plaintext.size());
    secure_clear(tag, TAG_SIZE);
    
    return true;
}

int main() {
    // Test case 1: Basic encryption and decryption
    {
        std::string base_dir = ".";
        std::string input = "test1.txt";
        std::string encrypted = "test1.enc";
        std::string decrypted = "test1_dec.txt";
        std::string passphrase = "StrongPassphrase123!";
        
        // Create test file
        std::ofstream ofs(input);
        ofs << "Hello, World! This is a test message.";
        ofs.close();
        
        if (encrypt_file(base_dir, input, encrypted, passphrase)) {
            std::cout << "Test 1: Encryption successful" << std::endl;
            if (decrypt_file(base_dir, encrypted, decrypted, passphrase)) {
                std::cout << "Test 1: Decryption successful" << std::endl;
            } else {
                std::cout << "Test 1: Decryption failed" << std::endl;
            }
        } else {
            std::cout << "Test 1: Encryption failed" << std::endl;
        }
    }
    
    // Test case 2: Wrong passphrase
    {
        std::string base_dir = ".";
        std::string input = "test2.txt";
        std::string encrypted = "test2.enc";
        std::string decrypted = "test2_dec.txt";
        
        std::ofstream ofs(input);
        ofs << "Secret data";
        ofs.close();
        
        encrypt_file(base_dir, input, encrypted, "CorrectPass123!");
        if (!decrypt_file(base_dir, encrypted, decrypted, "WrongPass123!")) {
            std::cout << "Test 2: Correctly rejected wrong passphrase" << std::endl;
        } else {
            std::cout << "Test 2: Failed - accepted wrong passphrase" << std::endl;
        }
    }
    
    // Test case 3: Large file
    {
        std::string base_dir = ".";
        std::string input = "test3.txt";
        std::string encrypted = "test3.enc";
        std::string decrypted = "test3_dec.txt";
        std::string passphrase = "LargeFileTest456!";
        
        std::ofstream ofs(input);
        for (int i = 0; i < 10000; i++) {
            ofs << "This is line " << i << " of the large test file.\\n";
        }
        ofs.close();
        
        if (encrypt_file(base_dir, input, encrypted, passphrase)) {
            std::cout << "Test 3: Large file encryption successful" << std::endl;
            if (decrypt_file(base_dir, encrypted, decrypted, passphrase)) {
                std::cout << "Test 3: Large file decryption successful" << std::endl;
            }
        }
    }
    
    // Test case 4: Empty passphrase rejection
    {
        std::string base_dir = ".";
        std::string input = "test4.txt";
        std::string encrypted = "test4.enc";
        
        std::ofstream ofs(input);
        ofs << "Test data";
        ofs.close();
        
        if (!encrypt_file(base_dir, input, encrypted, "")) {
            std::cout << "Test 4: Correctly rejected empty passphrase" << std::endl;
        }
    }
    
    // Test case 5: Path traversal prevention
    {
        std::string base_dir = ".";
        std::string input = "../etc/passwd";
        std::string encrypted = "test5.enc";
        std::string passphrase = "PathTest789!";
        
        if (!encrypt_file(base_dir, input, encrypted, passphrase)) {
            std::cout << "Test 5: Correctly rejected path traversal attempt" << std::endl;
        }
    }
    
    return 0;
}
