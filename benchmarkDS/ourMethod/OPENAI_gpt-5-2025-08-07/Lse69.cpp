#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <regex>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <random>
#include <iostream>
#include <sys/sendfile.h>
#include <dirent.h>

static const size_t MAX_SIZE = 8 * 1024 * 1024; // 8 MiB

static bool is_safe_filename(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) return false;
    static const std::regex re("^[A-Za-z0-9._-]{1,255}$");
    return std::regex_match(name, re);
}

static std::string base64_encode(const std::vector<unsigned char>& data) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t in_len = data.size();
    std::string out;
    out.reserve(((in_len + 2) / 3) * 4);
    size_t i = 0;
    while (i + 2 < in_len) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i < in_len) {
        unsigned int n = data[i] << 16;
        if (i + 1 < in_len) n |= (data[i+1] << 8);
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        if (i + 1 < in_len) {
            out.push_back(tbl[(n >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

static bool ensure_dir(const std::string& dir) {
    struct stat st;
    if (stat(dir.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir(dir.c_str(), 0700) == 0) return true;
    return false;
}

static std::string upload_image(const std::string& base_dir,
                                const std::string& input_name,
                                const std::string& output_name) {
    if (!is_safe_filename(input_name) || !is_safe_filename(output_name)) {
        return std::string();
    }
    if (!ensure_dir(base_dir)) {
        return std::string();
    }

    int dfd = open(base_dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) {
        return std::string();
    }

    int ifd = openat(dfd, input_name.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (ifd < 0) {
        close(dfd);
        return std::string();
    }

    struct stat st;
    if (fstat(ifd, &st) != 0) {
        close(ifd);
        close(dfd);
        return std::string();
    }
    if (!S_ISREG(st.st_mode)) {
        close(ifd);
        close(dfd);
        return std::string();
    }
    if (st.st_size < 0 || static_cast<size_t>(st.st_size) > MAX_SIZE) {
        close(ifd);
        close(dfd);
        return std::string();
    }

    std::vector<unsigned char> buf;
    buf.resize(static_cast<size_t>(st.st_size));
    size_t to_read = buf.size();
    size_t off = 0;
    while (to_read > 0) {
        ssize_t r = read(ifd, buf.data() + off, to_read);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(ifd);
            close(dfd);
            return std::string();
        }
        if (r == 0) break;
        off += static_cast<size_t>(r);
        to_read -= static_cast<size_t>(r);
    }
    if (off != buf.size()) {
        close(ifd);
        close(dfd);
        return std::string();
    }
    close(ifd);

    std::string encoded = base64_encode(buf);

    // Create unique temp file name in base_dir
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;
    std::string tmpname;
    int tfd = -1;
    for (int i = 0; i < 10; ++i) {
        char tmpbuf[64];
        snprintf(tmpbuf, sizeof(tmpbuf), "enc-%016llx.tmp", (unsigned long long)dist(gen));
        tmpname = tmpbuf;
        tfd = openat(dfd, tmpname.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (tfd >= 0) break;
        if (errno != EEXIST) {
            // some other error
            close(dfd);
            return std::string();
        }
    }
    if (tfd < 0) {
        close(dfd);
        return std::string();
    }

    // Write encoded data
    const char* p = encoded.c_str();
    size_t left = encoded.size();
    while (left > 0) {
        ssize_t w = write(tfd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(tfd);
            unlinkat(dfd, tmpname.c_str(), 0);
            close(dfd);
            return std::string();
        }
        p += w;
        left -= static_cast<size_t>(w);
    }
    // fsync file
    if (fsync(tfd) != 0) {
        close(tfd);
        unlinkat(dfd, tmpname.c_str(), 0);
        close(dfd);
        return std::string();
    }
    if (close(tfd) != 0) {
        unlinkat(dfd, tmpname.c_str(), 0);
        close(dfd);
        return std::string();
    }

    // Avoid overwrite using link+unlink for atomic publish
    if (linkat(dfd, tmpname.c_str(), dfd, output_name.c_str(), 0) != 0) {
        // destination exists or other error
        unlinkat(dfd, tmpname.c_str(), 0);
        close(dfd);
        return std::string();
    }
    // Remove temp name
    if (unlinkat(dfd, tmpname.c_str(), 0) != 0) {
        // best effort
    }

    // fsync directory
    fsync(dfd);
    close(dfd);

    return base_dir + "/" + output_name;
}

static bool write_sample_file(const std::string& base_dir, const std::string& name, const std::vector<unsigned char>& content) {
    if (!is_safe_filename(name)) return false;
    int dfd = open(base_dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) return false;

    // Create temp
    char tmpname[64];
    snprintf(tmpname, sizeof(tmpname), "in-%ld.tmp", (long)getpid());
    int tfd = openat(dfd, tmpname, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (tfd < 0) { close(dfd); return false; }
    size_t left = content.size();
    const unsigned char* p = content.data();
    while (left > 0) {
        ssize_t w = write(tfd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(tfd); unlinkat(dfd, tmpname, 0); close(dfd); return false;
        }
        p += w; left -= (size_t)w;
    }
    fsync(tfd);
    close(tfd);

    // Publish atomically: fail if exists
    if (linkat(dfd, tmpname, dfd, name.c_str(), 0) != 0) {
        unlinkat(dfd, tmpname, 0); close(dfd); return false;
    }
    unlinkat(dfd, tmpname, 0);
    fsync(dfd);
    close(dfd);
    return true;
}

int main() {
    std::string base = "base_cpp";
    if (!ensure_dir(base)) {
        std::cout << "Setup failed\n";
        return 0;
    }
    // Create 5 sample files
    std::vector<unsigned char> c1(32), c2(64), c3(128), c4(16), c5(256);
    std::random_device rd;
    for (auto& b : c1) b = (unsigned char)rd();
    for (auto& b : c2) b = (unsigned char)rd();
    for (auto& b : c3) b = (unsigned char)rd();
    for (auto& b : c4) b = (unsigned char)rd();
    for (auto& b : c5) b = (unsigned char)rd();
    write_sample_file(base, "img1.bin", c1);
    write_sample_file(base, "img2.jpg", c2);
    write_sample_file(base, "img3.png", c3);
    write_sample_file(base, "icon4.gif", c4);
    write_sample_file(base, "photo5.bmp", c5);

    const char* inputs[5] = {"img1.bin", "img2.jpg", "img3.png", "icon4.gif", "photo5.bmp"};
    const char* outputs[5] = {"img1.b64", "img2.b64", "img3.b64", "icon4.b64", "photo5.b64"};
    for (int i = 0; i < 5; ++i) {
        std::string res = upload_image(base, inputs[i], outputs[i]);
        if (!res.empty()) {
            std::cout << "OK: " << res << "\n";
        } else {
            std::cout << "FAIL\n";
        }
    }
    return 0;
}