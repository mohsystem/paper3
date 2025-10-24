#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

size_t xor_transform_file(const std::string& input_path, const std::string& output_path, const std::vector<uint8_t>& key) {
    if (key.empty()) {
        throw std::invalid_argument("Key must not be empty");
    }
    std::ifstream in(input_path, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open input");
    std::ofstream out(output_path, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open output");

    const size_t BUF = 65536;
    std::vector<uint8_t> buf(BUF);
    size_t total = 0;
    size_t ki = 0;
    while (in) {
        in.read(reinterpret_cast<char*>(buf.data()), buf.size());
        std::streamsize got = in.gcount();
        if (got <= 0) break;
        for (std::streamsize i = 0; i < got; ++i) {
            buf[static_cast<size_t>(i)] ^= key[ki];
            ki++;
            if (ki == key.size()) ki = 0;
        }
        out.write(reinterpret_cast<char*>(buf.data()), got);
        total += static_cast<size_t>(got);
    }
    return total;
}

size_t encrypt_file(const std::string& input_path, const std::string& output_path, const std::string& key) {
    std::vector<uint8_t> k(key.begin(), key.end());
    return xor_transform_file(input_path, output_path, k);
}

size_t decrypt_file(const std::string& input_path, const std::string& output_path, const std::string& key) {
    std::vector<uint8_t> k(key.begin(), key.end());
    return xor_transform_file(input_path, output_path, k);
}

void write_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

std::vector<uint8_t> read_bytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::vector<uint8_t> data;
    if (!in) return data;
    in.seekg(0, std::ios::end);
    std::streampos sz = in.tellg();
    in.seekg(0, std::ios::beg);
    data.resize(static_cast<size_t>(sz));
    in.read(reinterpret_cast<char*>(data.data()), sz);
    return data;
}

bool files_equal(const std::string& a, const std::string& b) {
    return read_bytes(a) == read_bytes(b);
}

int main() {
    try {
        // Test 1: simple text
        write_bytes("cpp_in1.txt", std::vector<uint8_t>{'H','e','l','l','o',',',' ','C','+','+','!'});
        encrypt_file("cpp_in1.txt", "cpp_in1.enc", "k1");
        decrypt_file("cpp_in1.enc", "cpp_in1.dec", "k1");
        std::cout << "Test1 OK: " << (files_equal("cpp_in1.txt", "cpp_in1.dec") ? "true" : "false") << "\n";

        // Test 2: unicode-ish bytes (UTF-8)
        std::string unicode = u8"Привет, мир 🌟";
        write_bytes("cpp_in2.txt", std::vector<uint8_t>(unicode.begin(), unicode.end()));
        encrypt_file("cpp_in2.txt", "cpp_in2.enc", "ключ");
        decrypt_file("cpp_in2.enc", "cpp_in2.dec", "ключ");
        std::cout << "Test2 OK: " << (files_equal("cpp_in2.txt", "cpp_in2.dec") ? "true" : "false") << "\n";

        // Test 3: larger content
        std::string big;
        big.reserve(1024 * 200);
        for (int i = 0; i < 20000; ++i) big += "Line " + std::to_string(i) + " - C++ stream test.\n";
        write_bytes("cpp_in3.txt", std::vector<uint8_t>(big.begin(), big.end()));
        encrypt_file("cpp_in3.txt", "cpp_in3.enc", "long_key_for_cpp_12345");
        decrypt_file("cpp_in3.enc", "cpp_in3.dec", "long_key_for_cpp_12345");
        std::cout << "Test3 OK: " << (files_equal("cpp_in3.txt", "cpp_in3.dec") ? "true" : "false") << "\n";

        // Test 4: binary bytes
        std::vector<uint8_t> bin4(4096);
        for (size_t i = 0; i < bin4.size(); ++i) bin4[i] = static_cast<uint8_t>(i & 0xFF);
        write_bytes("cpp_in4.bin", bin4);
        encrypt_file("cpp_in4.bin", "cpp_in4.enc", std::string("\x01\x02\x03\x04delta", 9));
        decrypt_file("cpp_in4.enc", "cpp_in4.dec", std::string("\x01\x02\x03\x04delta", 9));
        std::cout << "Test4 OK: " << (files_equal("cpp_in4.bin", "cpp_in4.dec") ? "true" : "false") << "\n";

        // Test 5: empty file
        write_bytes("cpp_in5.empty", std::vector<uint8_t>{});
        encrypt_file("cpp_in5.empty", "cpp_in5.enc", "empty");
        decrypt_file("cpp_in5.enc", "cpp_in5.dec", "empty");
        std::cout << "Test5 OK: " << (files_equal("cpp_in5.empty", "cpp_in5.dec") ? "true" : "false") << "\n";

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}