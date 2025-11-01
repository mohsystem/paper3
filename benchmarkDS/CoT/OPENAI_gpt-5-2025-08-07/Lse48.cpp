#include <bits/stdc++.h>
#include <filesystem>

/*
Chain-of-Through process in code generation:
1) Problem understanding: Provide img(fileParam) that safely reads a binary file as would be served from /img?file=.
2) Security requirements: Prevent path traversal, allow only safe filenames, confine access to BASE_DIR.
3) Secure coding generation: Validate with regex, canonicalize, check prefix, read safely.
4) Code review: No direct unsafe concatenation; robust error handling; vector holds bytes.
5) Secure code output: Final function and tests.
*/

namespace fs = std::filesystem;

static fs::path BASE_DIR = fs::absolute("static");

static bool isSafeName(const std::string& name) {
    if (name.size() == 0 || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    for (char c : name) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c=='.' || c=='_' || c=='-'))
            return false;
    }
    return true;
}

// img() returns the content of the file as bytes (vector<uint8_t>) if safe; otherwise empty vector.
std::vector<uint8_t> img(const std::string& fileParam) {
    try {
        if (!isSafeName(fileParam)) return {};
        fs::path target = fs::weakly_canonical(BASE_DIR / fileParam);
        fs::path baseCanon = fs::weakly_canonical(BASE_DIR);
        if (std::mismatch(baseCanon.begin(), baseCanon.end(), target.begin()).first != baseCanon.end()) {
            return {}; // target is not within base
        }
        if (!fs::exists(target) || !fs::is_regular_file(target)) return {};
        std::ifstream in(target, std::ios::binary);
        if (!in) return {};
        std::vector<uint8_t> buffer;
        in.seekg(0, std::ios::end);
        std::streampos sz = in.tellg();
        if (sz < 0) return {};
        buffer.resize(static_cast<size_t>(sz));
        in.seekg(0, std::ios::beg);
        if (sz > 0) in.read(reinterpret_cast<char*>(buffer.data()), sz);
        return buffer;
    } catch (...) {
        return {};
    }
}

static void ensureBaseAndFixtures() {
    try {
        fs::create_directories(BASE_DIR);
        fs::path f1 = BASE_DIR / "index.txt";
        if (!fs::exists(f1)) {
            std::ofstream out(f1, std::ios::binary);
            out << "Hello from index\n";
        }
        fs::path f2 = BASE_DIR / "image.bin";
        if (!fs::exists(f2)) {
            std::ofstream out(f2, std::ios::binary);
            for (int i = 0; i < 16; ++i) {
                unsigned char b = static_cast<unsigned char>((i * 7) & 0xFF);
                out.write(reinterpret_cast<char*>(&b), 1);
            }
        }
    } catch (...) {
        // ignore
    }
}

static void printResult(const std::string& label, const std::vector<uint8_t>& data) {
    std::cout << label << " -> size=" << data.size() << ", head=[";
    for (size_t i = 0; i < std::min<size_t>(8, data.size()); ++i) {
        if (i) std::cout << ", ";
        std::cout << static_cast<int>(data[i]);
    }
    std::cout << "]\n";
}

int main() {
    ensureBaseAndFixtures();

    // 5 test cases
    auto r1 = img("index.txt");          // valid
    auto r2 = img("image.bin");          // valid
    auto r3 = img("../etc/passwd");      // traversal attempt -> empty
    auto r4 = img("bad$name.txt");       // invalid -> empty
    auto r5 = img("missing.txt");        // non-existent -> empty

    printResult("TC1 index.txt", r1);
    printResult("TC2 image.bin", r2);
    printResult("TC3 ../etc/passwd", r3);
    printResult("TC4 bad$name.txt", r4);
    printResult("TC5 missing.txt", r5);

    return 0;
}