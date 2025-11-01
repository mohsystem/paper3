#include <iostream>
#include <fstream>
#include <vector>
#include <string>

static const std::string B64CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const unsigned char* data, size_t len) {
    std::string out;
    out.reserve(4 * ((len + 2) / 3));
    unsigned int val = 0;
    int valb = -6;
    for (size_t i = 0; i < len; ++i) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            out.push_back(B64CHARS[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(B64CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::string upload_image(const std::string& srcPath, const std::string& outBase64Path) {
    std::ifstream in(srcPath, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open input file: " + srcPath);
    }
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::string encoded = base64_encode(buffer.data(), buffer.size());

    std::ofstream out(outBase64Path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to open output file: " + outBase64Path);
    }
    out.write(encoded.data(), static_cast<std::streamsize>(encoded.size()));
    return encoded;
}

void write_bytes(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

int main() {
    try {
        std::vector<std::string> srcs = {
            "cpp_img1.bin", "cpp_img2.bin", "cpp_img3.bin", "cpp_img4.bin", "cpp_img5.bin"
        };
        std::vector<std::string> outs = {
            "cpp_img1.b64", "cpp_img2.b64", "cpp_img3.b64", "cpp_img4.b64", "cpp_img5.b64"
        };
        std::vector<std::vector<unsigned char>> samples = {
            {0,1,2,3,4,5,6,7,8,9},
            {0xFF,0xD8,0xFF,0xE0,0x00,0x10,0x4A,0x46,0x49,0x46},
            {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A},
            {'C','+','+',' ','B','a','s','e','6','4'},
            {0x00,0xFF,0xAA,0x55,0xCC,0x33,0x77,0x88,0x99}
        };

        for (size_t i = 0; i < 5; ++i) {
            write_bytes(srcs[i], samples[i]);
            std::string b64 = upload_image(srcs[i], outs[i]);
            std::cout << "Test " << (i+1) << ": src=" << srcs[i]
                      << ", out=" << outs[i] << ", b64_len=" << b64.size() << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}