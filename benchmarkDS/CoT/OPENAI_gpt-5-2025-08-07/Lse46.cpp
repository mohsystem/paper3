#include <bits/stdc++.h>
using namespace std;

static const unordered_set<string> ALLOWED_EXT = {
    "png","jpg","jpeg","gif","webp","bmp","svg"
};

static string imagesDir() {
    char cwd[4096];
    if (!getcwd(cwd, sizeof(cwd))) return string("images");
    return string(cwd) + string("/") + string("images");
}

static bool ends_with_icase(const string& s, const string& suf) {
    if (s.size() < suf.size()) return false;
    for (size_t i = 0; i < suf.size(); ++i) {
        char a = tolower(s[s.size()-suf.size()+i]);
        char b = tolower(suf[i]);
        if (a != b) return false;
    }
    return true;
}

static bool is_valid_name(const string& name) {
    if (name.empty()) return false;
    // basename only
    if (name.find('/') != string::npos || name.find('\\') != string::npos) return false;
    if (name.find("..") != string::npos) return false;
    auto pos = name.find_last_of('.');
    if (pos == string::npos || pos == 0 || pos == name.size()-1) return false;
    string ext = name.substr(pos+1);
    for (auto& c : ext) c = (char)tolower(c);
    return ALLOWED_EXT.count(ext) > 0;
}

static string guess_mime(const string& base) {
    string lower;
    lower.reserve(base.size());
    for (char c : base) lower.push_back((char)tolower((unsigned char)c));
    if (ends_with_icase(lower, ".png")) return "image/png";
    if (ends_with_icase(lower, ".jpg") || ends_with_icase(lower, ".jpeg")) return "image/jpeg";
    if (ends_with_icase(lower, ".gif")) return "image/gif";
    if (ends_with_icase(lower, ".webp")) return "image/webp";
    if (ends_with_icase(lower, ".bmp")) return "image/bmp";
    if (ends_with_icase(lower, ".svg")) return "image/svg+xml";
    return "application/octet-stream";
}

// Function that accepts the input as parameter and returns an HTTP-like response string
static string img(const string& name) {
    if (!is_valid_name(name)) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
    }
    string base = name; // already ensured no path parts
    string dir = imagesDir();
    string path = dir + "/" + base;

    // Ensure dir exists in a safe manner; no actual path normalization in standard C++
    // We rely on earlier validations.
    ifstream in(path, ios::binary);
    if (!in.good()) {
        return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }
    vector<char> buf((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    string mime = guess_mime(base);
    ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: " << mime << "\r\n";
    oss << "Content-Length: " << buf.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    string head = oss.str();
    string resp;
    resp.reserve(head.size() + buf.size());
    resp.append(head);
    resp.append(buf.data(), buf.size());
    return resp;
}

static void create_sample_files() {
    string dir = imagesDir();
    // create directory if not exists
#ifdef _WIN32
    _mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
    // sample.png
    {
        string p = dir + "/sample.png";
        FILE* f = fopen(p.c_str(), "rb");
        if (!f) {
            f = fopen(p.c_str(), "wb");
            if (f) {
                unsigned char png[] = {0x89, 'P','N','G', 0x0D,0x0A,0x1A,0x0A, 0x00};
                fwrite(png, 1, sizeof(png), f);
                fclose(f);
            }
        } else {
            fclose(f);
        }
    }
    // logo.jpg
    {
        string p = dir + "/logo.jpg";
        FILE* f = fopen(p.c_str(), "rb");
        if (!f) {
            f = fopen(p.c_str(), "wb");
            if (f) {
                unsigned char jpg[] = {0xFF,0xD8,0xFF, 0x00, 0x11, 0x22};
                fwrite(jpg, 1, sizeof(jpg), f);
                fclose(f);
            }
        } else {
            fclose(f);
        }
    }
}

int main() {
    create_sample_files();
    vector<string> tests = {
        "sample.png",
        "logo.jpg",
        "../secret.png",
        "notfound.png",
        "subdir/evil.png"
    };
    for (auto& t : tests) {
        string resp = img(t);
        // print status line and content length
        auto pos = resp.find("\r\n");
        string status = (pos != string::npos) ? resp.substr(0, pos) : "(no status line)";
        size_t clenPos = resp.find("Content-Length:");
        string clen = "N/A";
        if (clenPos != string::npos) {
            size_t end = resp.find("\r\n", clenPos);
            if (end != string::npos) clen = resp.substr(clenPos, end - clenPos);
        }
        cout << "Test name=" << t << " -> " << status << ", " << clen << "\n";
    }
    return 0;
}