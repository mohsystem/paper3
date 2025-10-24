#include <algorithm>
#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>

struct FileStats {
    std::string path;
    uint64_t bytes;
    uint64_t lines;
    bool success;
    std::string message;
};

static bool get_realpath(const std::string &in, std::string &out) {
    char buf[PATH_MAX];
    if (realpath(in.c_str(), buf) == nullptr) {
        return false;
    }
    out.assign(buf);
    return true;
}

static bool is_within_dir(const std::string &base_real, const std::string &path_real) {
    std::string base = base_real;
    if (!base.empty() && base.back() != '/') base.push_back('/');
    if (path_real.size() < base.size()) return false;
    return std::equal(base.begin(), base.end(), path_real.begin());
}

static bool try_exclusive_lock(int fd) {
    struct flock fl;
    std::memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0; // entire file
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            return false;
        }
        return false;
    }
    return true;
}

static bool write_atomic_sidecar(const std::string &final_path, const std::string &content) {
    // Create a temp file in the same directory and rename atomically.
    std::size_t slash = final_path.find_last_of('/');
    std::string dir = (slash == std::string::npos) ? "." : final_path.substr(0, slash);
    std::string name = (slash == std::string::npos) ? final_path : final_path.substr(slash + 1);
    std::string tmpl = dir + "/.tmp-" + name + "-XXXXXX";
    std::vector<char> tpl(tmpl.begin(), tmpl.end());
    tpl.push_back('\0');

    int tmpfd = mkstemp(tpl.data());
    if (tmpfd == -1) {
        return false;
    }
    // Restrict permissions: 0644
    fchmod(tmpfd, 0644);

    const char *data = content.c_str();
    size_t left = content.size();
    ssize_t w = 0;
    while (left > 0) {
        w = write(tmpfd, data, left > 65536 ? 65536 : left);
        if (w == -1) {
            if (errno == EINTR) continue;
            close(tmpfd);
            unlink(tpl.data());
            return false;
        }
        data += w;
        left -= static_cast<size_t>(w);
    }
    if (fsync(tmpfd) == -1) {
        close(tmpfd);
        unlink(tpl.data());
        return false;
    }
    if (close(tmpfd) == -1) {
        unlink(tpl.data());
        return false;
    }
    if (rename(tpl.data(), final_path.c_str()) == -1) {
        unlink(tpl.data());
        return false;
    }
    return true;
}

static FileStats process_one_file(const std::string &base_real, const std::string &file_path) {
    FileStats st;
    st.path = file_path;
    st.bytes = 0;
    st.lines = 0;
    st.success = false;
    st.message.clear();

    // Validate canonical path remains within base and is a regular file and not a symlink.
    struct stat lst;
    if (lstat(file_path.c_str(), &lst) == -1) {
        st.message = "lstat failed";
        return st;
    }
    if (!S_ISREG(lst.st_mode)) {
        st.message = "not a regular file";
        return st;
    }
    if ((lst.st_mode & S_IFMT) != S_IFREG) {
        st.message = "not a regular file";
        return st;
    }
    // Resolve realpath and ensure containment
    std::string realp;
    if (!get_realpath(file_path, realp)) {
        st.message = "realpath failed";
        return st;
    }
    if (!is_within_dir(base_real, realp)) {
        st.message = "path escapes base dir";
        return st;
    }

    int fd = open(file_path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        st.message = "open failed";
        return st;
    }
    bool locked = try_exclusive_lock(fd);
    if (!locked) {
        close(fd);
        st.message = "lock busy";
        return st;
    }

    // Process: count bytes and lines in chunks
    const size_t BUFSZ = 1 << 16;
    std::vector<char> buf(BUFSZ);
    while (true) {
        ssize_t r = read(fd, buf.data(), BUFSZ);
        if (r == -1) {
            if (errno == EINTR) continue;
            st.message = "read failed";
            close(fd);
            return st;
        }
        if (r == 0) break;
        st.bytes += static_cast<uint64_t>(r);
        for (ssize_t i = 0; i < r; ++i) {
            if (buf[static_cast<size_t>(i)] == '\n') st.lines++;
        }
    }
    // Release lock by closing
    if (close(fd) == -1) {
        st.message = "close failed";
        return st;
    }

    // Prepare sidecar output content and write atomically
    std::ostringstream oss;
    oss << "file: " << realp << "\n";
    oss << "bytes: " << st.bytes << "\n";
    oss << "lines: " << st.lines << "\n";
    std::string out_path = file_path + ".out";

    if (!write_atomic_sidecar(out_path, oss.str())) {
        st.message = "atomic write failed";
        return st;
    }
    st.success = true;
    st.message = "ok";
    return st;
}

class ThreadSafeQueue {
public:
    void push(const std::string &s) {
        std::unique_lock<std::mutex> lk(mu_);
        q_.push_back(s);
        cv_.notify_one();
    }
    bool pop(std::string &out) {
        std::unique_lock<std::mutex> lk(mu_);
        while (q_.empty() && !closed_) {
            cv_.wait(lk);
        }
        if (q_.empty()) return false;
        out = std::move(q_.front());
        q_.pop_front();
        return true;
    }
    void close() {
        std::unique_lock<std::mutex> lk(mu_);
        closed_ = true;
        cv_.notify_all();
    }
private:
    std::deque<std::string> q_;
    std::mutex mu_;
    std::condition_variable cv_;
    bool closed_ = false;
};

static bool list_regular_files(const std::string &base_dir, std::vector<std::string> &out, std::string &base_real) {
    out.clear();
    if (!get_realpath(base_dir, base_real)) {
        return false;
    }
    DIR *dir = opendir(base_real.c_str());
    if (!dir) return false;

    struct dirent *ent = nullptr;
    while ((ent = readdir(dir)) != nullptr) {
        if (std::strcmp(ent->d_name, ".") == 0 || std::strcmp(ent->d_name, "..") == 0) continue;
        std::string candidate = base_real + "/" + std::string(ent->d_name);
        struct stat st;
        if (lstat(candidate.c_str(), &st) == -1) continue;
        if (!S_ISREG(st.st_mode)) continue; // skip symlinks, dirs, etc.
        // Confirm containment after realpath
        std::string rp;
        if (!get_realpath(candidate, rp)) continue;
        if (!is_within_dir(base_real, rp)) continue;
        out.push_back(rp);
    }
    closedir(dir);
    return true;
}

struct ProcessResult {
    int processed;
    int skipped;
};

static ProcessResult process_directory_concurrently(const std::string &base_dir, size_t threads, std::vector<FileStats> &stats_out) {
    stats_out.clear();
    std::string base_real;
    std::vector<std::string> files;
    ProcessResult res{0, 0};
    if (!list_regular_files(base_dir, files, base_real)) {
        return res;
    }

    ThreadSafeQueue q;
    for (const auto &p : files) q.push(p);
    q.close();

    std::mutex stats_mu;
    std::vector<std::thread> workers;
    for (size_t i = 0; i < std::max<size_t>(1, threads); ++i) {
        workers.emplace_back([&]() {
            std::string path;
            while (q.pop(path)) {
                FileStats fs = process_one_file(base_real, path);
                {
                    std::lock_guard<std::mutex> lk(stats_mu);
                    stats_out.push_back(fs);
                }
            }
        });
    }
    for (auto &t : workers) t.join();

    for (const auto &s : stats_out) {
        if (s.success) res.processed++;
        else res.skipped++;
    }
    return res;
}

// Test helpers
static bool mkdir_p(const std::string &path, mode_t mode) {
    if (mkdir(path.c_str(), mode) == 0) return true;
    if (errno == EEXIST) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
    }
    return false;
}

static bool write_file_atomic(const std::string &path, const std::string &content) {
    std::size_t slash = path.find_last_of('/');
    std::string dir = (slash == std::string::npos) ? "." : path.substr(0, slash);
    std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    std::string tmpl = dir + "/.gen-" + name + "-XXXXXX";
    std::vector<char> tpl(tmpl.begin(), tmpl.end());
    tpl.push_back('\0');
    int fd = mkstemp(tpl.data());
    if (fd == -1) return false;
    fchmod(fd, 0644);
    const char *p = content.c_str();
    size_t left = content.size();
    while (left > 0) {
        ssize_t w = write(fd, p, left > 65536 ? 65536 : left);
        if (w == -1) {
            if (errno == EINTR) continue;
            close(fd);
            unlink(tpl.data());
            return false;
        }
        p += w;
        left -= (size_t)w;
    }
    if (fsync(fd) == -1) {
        close(fd);
        unlink(tpl.data());
        return false;
    }
    if (close(fd) == -1) {
        unlink(tpl.data());
        return false;
    }
    if (rename(tpl.data(), path.c_str()) == -1) {
        unlink(tpl.data());
        return false;
    }
    return true;
}

static std::string make_temp_dir(const std::string &prefix) {
    std::string base = "/tmp/" + prefix + "-XXXXXX";
    std::vector<char> tpl(base.begin(), base.end());
    tpl.push_back('\0');
    char *d = mkdtemp(tpl.data());
    if (!d) return "";
    return std::string(d);
}

static bool create_symlink_safe(const std::string &target, const std::string &linkpath) {
    // Best effort; ignore failures in tests
    unlink(linkpath.c_str());
    return symlink(target.c_str(), linkpath.c_str()) == 0;
}

static void create_test_files(const std::string &dir, int count, size_t size_each, bool with_newlines) {
    for (int i = 0; i < count; ++i) {
        std::ostringstream p;
        p << dir << "/file_" << i << ".txt";
        std::string content;
        content.reserve(size_each);
        for (size_t j = 0; j < size_each; ++j) {
            char c = 'a' + (char)((i + j) % 26);
            content.push_back(c);
            if (with_newlines && j % 64 == 63) content.push_back('\n');
        }
        write_file_atomic(p.str(), content);
    }
}

int main() {
    // 5 test cases
    // Test 1: Basic processing of 5 small files
    std::string d1 = make_temp_dir("cfp1");
    mkdir_p(d1, 0700);
    create_test_files(d1, 5, 1024, true);
    std::vector<FileStats> stats1;
    ProcessResult r1 = process_directory_concurrently(d1, 4, stats1);
    std::cout << "Test1 processed=" << r1.processed << " skipped=" << r1.skipped << "\n";

    // Test 2: Directory with a symlink (should be skipped)
    std::string d2 = make_temp_dir("cfp2");
    mkdir_p(d2, 0700);
    write_file_atomic(d2 + "/real.txt", "hello\nworld\n");
    create_symlink_safe(d2 + "/real.txt", d2 + "/link.txt");
    std::vector<FileStats> stats2;
    ProcessResult r2 = process_directory_concurrently(d2, 3, stats2);
    std::cout << "Test2 processed=" << r2.processed << " skipped=" << r2.skipped << "\n";

    // Test 3: Directory with subdirectory and regular files
    std::string d3 = make_temp_dir("cfp3");
    mkdir_p(d3, 0700);
    mkdir_p(d3 + "/sub", 0700);
    write_file_atomic(d3 + "/a.txt", std::string(5000, 'x'));
    write_file_atomic(d3 + "/sub/b.txt", "should not be seen");
    std::vector<FileStats> stats3;
    ProcessResult r3 = process_directory_concurrently(d3, 2, stats3);
    std::cout << "Test3 processed=" << r3.processed << " skipped=" << r3.skipped << "\n";

    // Test 4: Larger files
    std::string d4 = make_temp_dir("cfp4");
    mkdir_p(d4, 0700);
    create_test_files(d4, 3, 300000, true); // ~300KB each
    std::vector<FileStats> stats4;
    ProcessResult r4 = process_directory_concurrently(d4, 6, stats4);
    std::cout << "Test4 processed=" << r4.processed << " skipped=" << r4.skipped << "\n";

    // Test 5: Re-run processing on same dir (should overwrite sidecars safely)
    std::vector<FileStats> stats5;
    ProcessResult r5 = process_directory_concurrently(d4, 4, stats5);
    std::cout << "Test5 processed=" << r5.processed << " skipped=" << r5.skipped << "\n";

    // Print one example output file content path for verification
    if (!stats1.empty()) {
        std::cout << "Example sidecar: " << stats1[0].path << ".out\n";
    }
    return 0;
}