#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

class Lse50 {
public:
    static bool extract_tar_as_from_zip_named(const std::string& archivePath, const std::string& destDir) {
        std::ifstream in(archivePath, std::ios::binary);
        if (!in.is_open()) return false;

        if (!make_dirs(destDir)) return false;

        const size_t BLOCK = 512;
        std::vector<char> header(BLOCK);
        std::vector<char> buffer(8192);

        while (true) {
            if (!read_exact(in, header.data(), BLOCK)) {
                // EOF reached cleanly
                break;
            }
            if (is_all_zero(header.data(), BLOCK)) {
                // end of archive; read next block and stop
                read_exact(in, header.data(), BLOCK);
                break;
            }

            std::string name = read_string(header.data(), 0, 100);
            std::string prefix = read_string(header.data(), 345, 155);
            if (!prefix.empty()) name = prefix + "/" + name;
            long long size = parse_octal(header.data(), 124, 12);
            char type = header[156];

            if (name.empty()) {
                // skip payload
                skip(in, round_up_512(size));
                continue;
            }

            // path traversal checks
            if (!path_is_safe(name)) {
                return false;
            }

            std::string outPath = join_path(destDir, name);

            if (type == '5') {
                if (!make_dirs(outPath)) return false;
            } else if (type == '0' || type == '\0') {
                // ensure parent directories
                std::string parent = dirname(outPath);
                if (!make_dirs(parent)) return false;

                std::ofstream out(outPath, std::ios::binary);
                if (!out.is_open()) return false;
                long long remaining = size;
                while (remaining > 0) {
                    size_t toRead = (size_t) std::min<long long>(buffer.size(), remaining);
                    in.read(buffer.data(), toRead);
                    std::streamsize n = in.gcount();
                    if (n <= 0) return false;
                    out.write(buffer.data(), n);
                    remaining -= n;
                }
                out.close();
                // skip padding
                long long pad = (512 - (size % 512)) % 512;
                if (pad > 0) skip(in, pad);
            } else {
                // skip unknown types
                skip(in, round_up_512(size));
            }
        }

        return true;
    }

private:
    static bool read_exact(std::ifstream& in, char* buf, size_t len) {
        in.read(buf, len);
        if ((size_t)in.gcount() < len) {
            return false;
        }
        return true;
    }

    static void skip(std::ifstream& in, long long n) {
        const size_t CHUNK = 8192;
        std::vector<char> tmp(CHUNK);
        while (n > 0) {
            size_t toRead = (size_t) std::min<long long>(CHUNK, n);
            in.read(tmp.data(), toRead);
            std::streamsize got = in.gcount();
            if (got <= 0) break;
            n -= got;
        }
    }

    static bool is_all_zero(const char* b, size_t len) {
        for (size_t i = 0; i < len; ++i) if (b[i] != 0) return false;
        return true;
    }

    static std::string read_string(const char* b, int off, int len) {
        int end = off + len;
        int i = off;
        for (; i < end; ++i) {
            if (b[i] == '\0') break;
        }
        // trim trailing spaces
        int j = i - 1;
        while (j >= off && (b[j] == ' ' || b[j] == '\0')) --j;
        if (j < off) return std::string();
        return std::string(b + off, (size_t)(j - off + 1));
    }

    static long long parse_octal(const char* b, int off, int len) {
        long long val = 0;
        int end = off + len;
        int i = off;
        while (i < end && (b[i] == ' ' || b[i] == '\0')) i++;
        for (; i < end; ++i) {
            char c = b[i];
            if (c == '\0' || c == ' ') break;
            if (c < '0' || c > '7') break;
            val = (val << 3) + (c - '0');
        }
        return val;
    }

    static long long round_up_512(long long size) {
        long long rem = size % 512;
        return rem == 0 ? size : size + (512 - rem);
    }

    static bool path_is_safe(const std::string& name) {
        if (name.empty()) return false;
        if (name[0] == '/') return false;
        if (name.find("..") != std::string::npos) return false;
        return true;
    }

    static std::string join_path(const std::string& a, const std::string& b) {
        if (a.empty()) return b;
        if (b.empty()) return a;
        if (a.back() == '/') return a + b;
        return a + "/" + b;
    }

    static std::string dirname(const std::string& path) {
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos) return std::string();
        return path.substr(0, pos);
    }

    static bool make_dirs(const std::string& path) {
        if (path.empty()) return true;
        if (path == "/") return true;
        std::string cur;
        size_t i = 0;
        if (!path.empty() && path[0] == '/') {
            cur = "/";
            i = 1;
        }
        while (i <= path.size()) {
            size_t pos = path.find('/', i);
            std::string part = path.substr(i, pos == std::string::npos ? std::string::npos : pos - i);
            if (!part.empty()) {
                if (cur == "/" || cur.empty()) cur += part;
                else cur += "/" + part;

                struct stat st{};
                if (stat(cur.c_str(), &st) != 0) {
                    if (mkdir(cur.c_str(), 0755) != 0) {
                        // race? try stat again
                        if (stat(cur.c_str(), &st) != 0) return false;
                    }
                } else if (!S_ISDIR(st.st_mode)) {
                    return false;
                }
            }
            if (pos == std::string::npos) break;
            i = pos + 1;
        }
        return true;
    }
};

int main() {
    std::vector<std::pair<std::string, std::string>> tests = {
        {"archive.zip", "/tmp/unpack"},
        {"archive.zip", "/tmp/unpack1"},
        {"archive.tar", "/tmp/unpack2"},
        {"nonexistent.zip", "/tmp/unpack3"},
        {"./archive.zip", "/tmp/unpack4"}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        bool ok = Lse50::extract_tar_as_from_zip_named(tests[i].first, tests[i].second);
        std::cout << "Test " << (i + 1) << " (" << tests[i].first << " -> " << tests[i].second << "): " << (ok ? "OK" : "FAIL") << std::endl;
    }
    return 0;
}