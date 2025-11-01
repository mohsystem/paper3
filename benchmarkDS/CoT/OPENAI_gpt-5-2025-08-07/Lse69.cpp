// Steps (Chain-of-Through):
// 1) Problem understanding: Provide upload_image() that reads an image, base64-encodes it, writes to a file.
// 2) Security requirements: Validate existence, size, extension, signature, and ensure output path is safe.
// 3) Secure coding generation: Use std::filesystem, robust I/O, and fail-fast validation.
// 4) Code review: Check for path traversal, large input, incorrect types.
// 5) Secure code output: Final code with test cases.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <cctype>
#include <unordered_set>

namespace fs = std::filesystem;
static const uint64_t MAX_SIZE = 10ULL * 1024ULL * 1024ULL; // 10 MB

static bool hasAllowedExtension(const std::string& name) {
    static const std::unordered_set<std::string> exts = {".png",".jpg",".jpeg",".gif",".bmp",".webp"};
    std::string lower;
    lower.reserve(name.size());
    for (unsigned char c : name) lower.push_back((char)std::tolower(c));
    for (const auto& ext : exts) {
        if (lower.size() >= ext.size() && lower.rfind(ext) == lower.size() - ext.size())
            return true;
    }
    return false;
}

static bool allowedSignature(const std::vector<unsigned char>& head) {
    if (head.size() < 12) return false;
    // PNG
    const unsigned char png[] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
    bool isPng = true;
    for (int i = 0; i < 8; ++i) if (head[i] != png[i]) { isPng = false; break; }
    if (isPng) return true;
    // JPEG
    if (head[0]==0xFF && head[1]==0xD8 && head[2]==0xFF) return true;
    // GIF
    if (head[0]=='G' && head[1]=='I' && head[2]=='F' && head[3]=='8' && (head[4]=='7' || head[4]=='9') && head[5]=='a') return true;
    // BMP
    if (head[0]=='B' && head[1]=='M') return true;
    // WEBP
    if (head[0]=='R' && head[1]=='I' && head[2]=='F' && head[3]=='F' && head[8]=='W' && head[9]=='E' && head[10]=='B' && head[11]=='P') return true;
    return false;
}

static std::string base64Encode(const std::vector<unsigned char>& data) {
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);
    size_t i = 0;
    while (i + 3 <= data.size()) {
        uint32_t n = (uint32_t(data[i]) << 16) | (uint32_t(data[i+1]) << 8) | (uint32_t(data[i+2]));
        out.push_back(b64[(n >> 18) & 63]);
        out.push_back(b64[(n >> 12) & 63]);
        out.push_back(b64[(n >> 6) & 63]);
        out.push_back(b64[n & 63]);
        i += 3;
    }
    if (i < data.size()) {
        uint32_t n = uint32_t(data[i]) << 16;
        out.push_back(b64[(n >> 18) & 63]);
        if (i + 1 < data.size()) {
            n |= (uint32_t(data[i+1]) << 8);
            out.push_back(b64[(n >> 12) & 63]);
            out.push_back(b64[(n >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back(b64[(n >> 12) & 63]);
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

// Base64 decode for test image creation
static std::vector<unsigned char> base64Decode(const std::string& s) {
    static int T[256];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 256; ++i) T[i] = -1;
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < (int)chars.size(); ++i) T[(unsigned char)chars[i]] = i;
        T[(unsigned char)'='] = -2;
        init = true;
    }
    std::vector<unsigned char> out;
    int val = 0, valb = -8;
    for (unsigned char c : s) {
        int t = T[c];
        if (t == -1) continue; // skip whitespace/invalid
        if (t == -2) { // padding
            break;
        }
        val = (val << 6) | t;
        valb += 6;
        if (valb >= 0) {
            out.push_back((unsigned char)((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string upload_image(const std::string& inputPath, const std::string& outputPath) {
    if (inputPath.empty() || outputPath.empty()) throw std::invalid_argument("Paths must not be empty");

    fs::path in = fs::path(inputPath);
    if (!fs::exists(in) || !fs::is_regular_file(in)) throw std::runtime_error("Input is not a regular file");
    uintmax_t size = fs::file_size(in);
    if (size == 0 || size > MAX_SIZE) throw std::runtime_error("Invalid file size");
    if (!hasAllowedExtension(in.filename().string())) throw std::runtime_error("Disallowed file extension");

    // Read header
    std::ifstream is(in, std::ios::binary);
    if (!is) throw std::runtime_error("Failed to open input");
    std::vector<unsigned char> head(12);
    is.read(reinterpret_cast<char*>(head.data()), (std::streamsize)head.size());
    if (is.gcount() < (std::streamsize)head.size()) throw std::runtime_error("File too small or truncated");
    if (!allowedSignature(head)) throw std::runtime_error("Invalid or unsupported image signature");

    // Read full file
    is.clear();
    is.seekg(0, std::ios::beg);
    std::vector<unsigned char> data;
    data.reserve((size_t)size);
    data.assign(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
    is.close();

    std::string encoded = base64Encode(data);

    // Safe output path
    fs::path out = fs::absolute(fs::path(outputPath)).lexically_normal();
    fs::path cwd = fs::current_path();
    fs::path outParent = out.has_parent_path() ? out.parent_path() : cwd;
    fs::create_directories(outParent);

    // Ensure out is within cwd
    fs::path canonOutParent = fs::weakly_canonical(outParent);
    fs::path canonCwd = fs::weakly_canonical(cwd);
    if (canonOutParent.string().rfind(canonCwd.string(), 0) != 0) {
        throw std::runtime_error("Output path escapes working directory");
    }

    fs::path tmp = out;
    tmp += ".tmp";
    {
        std::ofstream os(tmp, std::ios::binary | std::ios::trunc);
        if (!os) throw std::runtime_error("Failed to open temp output");
        os.write(encoded.data(), (std::streamsize)encoded.size());
        if (!os.good()) throw std::runtime_error("Failed to write temp output");
    }
    fs::rename(tmp, out);

    return encoded;
}

static void writeBytes(const fs::path& p, const std::vector<unsigned char>& b) {
    fs::create_directories(p.parent_path());
    std::ofstream os(p, std::ios::binary | std::ios::trunc);
    if (!os) throw std::runtime_error("Failed to write test file");
    os.write(reinterpret_cast<const char*>(b.data()), (std::streamsize)b.size());
}

int main() {
    try {
        // Prepare 1x1 PNG
        std::string pngB64 = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMBAe0BqZcAAAAASUVORK5CYII=";
        std::vector<unsigned char> png = base64Decode(pngB64);

        fs::path inDir = fs::absolute("test_inputs_cpp");
        fs::path outDir = fs::absolute("encoded_outputs_cpp");
        fs::create_directories(inDir);
        fs::create_directories(outDir);

        std::string inNames[5]  = {"img1.png","img2.PNG","img3.pNg","img4.png","img5.png"};
        std::string outNames[5] = {"img1.b64","img2.b64","img3.b64","img4.b64","img5.b64"};

        for (int i = 0; i < 5; ++i) {
            writeBytes(inDir / inNames[i], png);
        }

        for (int i = 0; i < 5; ++i) {
            std::string enc = upload_image((inDir / inNames[i]).string(), (outDir / outNames[i]).string());
            std::cout << "Test " << (i+1) << " -> output: " << (outDir / outNames[i]) << " (encoded length=" << enc.size() << ")\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}