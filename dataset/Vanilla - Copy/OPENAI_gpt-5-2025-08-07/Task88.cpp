#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstdlib>
#include <chrono>

static std::string utf8_encode_cp(int cp) {
    std::string out;
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

static std::string get_temp_dir() {
    const char* env =
#ifdef _WIN32
        std::getenv("TEMP");
#else
        std::getenv("TMPDIR");
#endif
    if (env && *env) return std::string(env);
#ifdef _WIN32
    return "C:\\Windows\\Temp";
#else
    return "/tmp";
#endif
}

std::string create_unicode_temp_file(int length) {
    std::vector<int> pool = {
        'A','B','C','x','y','z','0','1','2','3',
        0x00E9, // é
        0x03B1, // α
        0x0416, // Ж
        0x4E2D, // 中
        0x6F22, // 漢
        0x3042, // あ
        0x20AC, // €
        0x1F642, // 🙂
        0x1F9E1  // 🧡
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);

    std::vector<int> cps;
    cps.reserve(length);
    for (int i = 0; i < length; ++i) cps.push_back(pool[dist(gen)]);

    std::string random_utf8;
    for (int cp : cps) random_utf8 += utf8_encode_cp(cp);

    std::ostringstream uni;
    for (size_t i = 0; i < cps.size(); ++i) {
        int cp = cps[i];
        int width = (cp <= 0xFFFF) ? 4 : 6;
        uni << "U+" << std::uppercase << std::setw(width) << std::setfill('0') << std::hex << cp;
        if (i + 1 < cps.size()) uni << ' ';
    }

    std::string content = random_utf8 + "\n" + uni.str();

    std::string dir = get_temp_dir();
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::uniform_int_distribution<long long> d2(0, 1'000'000'000);
    long long r = d2(gen);
    std::ostringstream name;
    name << "task88_" << now << "_" << r << ".txt";
    std::string path = dir;
    if (!path.empty() && path.back() != sep) path.push_back(sep);
    path += name.str();

    std::ofstream ofs(path, std::ios::binary);
    ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
    ofs.close();

    return path;
}

int main() {
    std::cout << create_unicode_temp_file(8) << std::endl;
    std::cout << create_unicode_temp_file(10) << std::endl;
    std::cout << create_unicode_temp_file(12) << std::endl;
    std::cout << create_unicode_temp_file(14) << std::endl;
    std::cout << create_unicode_temp_file(16) << std::endl;
    return 0;
}