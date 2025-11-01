// Build: g++ -std=c++17 -O2 -pthread server.cpp -o server
// Note: MySQL connectivity is not compiled by default. To enable, integrate MySQL C API and replace stub in query_max_temperature.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static const int PORT = 8083;
static const std::string ENDPOINT = "/api/temperature_for_location";
static const std::regex GRIB_ALLOWED("^[A-Za-z0-9_.-]{1,64}$");

struct DbResult {
    bool success;
    std::optional<double> max_temperature;
    std::string error; // "database_not_configured", "not_found", "internal_error"
};

static bool is_number(const std::string& s) {
    if (s.empty() || s.size() > 64) return false;
    bool dot=false, sign=false, digit=false;
    for (size_t i=0;i<s.size();++i){
        char c=s[i];
        if (c=='+'||c=='-'){ if (i!=0||sign) return false; sign=true; }
        else if (c=='.'){ if (dot) return false; dot=true; }
        else if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        else digit=true;
    }
    return digit;
}

static std::optional<double> parse_double_in_range(const std::string& s, double mn, double mx) {
    if (!is_number(s)) return std::nullopt;
    try {
        double v = std::stod(s);
        if (v >= mn && v <= mx) return v;
    } catch (...) {}
    return std::nullopt;
}
static std::optional<int> parse_int_in_range(const std::string& s, int mn, int mx) {
    if (s.empty() || s.size()>16) return std::nullopt;
    for (char c: s) if (!std::isdigit(static_cast<unsigned char>(c))) return std::nullopt;
    try {
        long v = std::stol(s);
        if (v >= mn && v <= mx) return static_cast<int>(v);
    } catch (...) {}
    return std::nullopt;
}

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i=0;i<s.size();++i) {
        if (s[i]=='%' && i+2<s.size() && std::isxdigit((unsigned char)s[i+1]) && std::isxdigit((unsigned char)s[i+2])) {
            std::string hex = s.substr(i+1,2);
            char ch = static_cast<char>(strtol(hex.c_str(), nullptr, 16));
            out.push_back(ch);
            i+=2;
        } else if (s[i]=='+') out.push_back(' ');
        else out.push_back(s[i]);
    }
    return out;
}

static std::map<std::string,std::string> parse_query(const std::string& q) {
    std::map<std::string,std::string> m;
    if (q.empty() || q.size()>2048) return m;
    size_t pos=0;
    while (pos<q.size()) {
        size_t amp = q.find('&', pos);
        std::string pair = q.substr(pos, amp==std::string::npos ? std::string::npos : amp-pos);
        size_t eq = pair.find('=');
        if (eq!=std::string::npos) {
            std::string k = url_decode(pair.substr(0, eq));
            std::string v = url_decode(pair.substr(eq+1));
            if (k.size()<=64 && v.size()<=256) m[k]=v;
        }
        if (amp==std::string::npos) break;
        pos = amp+1;
    }
    return m;
}

// Stub for DB query: returns error "database_not_configured" unless environment variables and MySQL integration are provided.
static DbResult query_max_temperature(double latitude, double longitude, int year, int month, int day, const std::string& grib_file) {
    const char* host = std::getenv("DB_HOST");
    const char* port = std::getenv("DB_PORT");
    const char* db   = std::getenv("DB_NAME");
    const char* user = std::getenv("DB_USER");
    const char* pass = std::getenv("DB_PASSWORD");
    if (!(host && port && db && user && pass)) {
        return {false, std::nullopt, "database_not_configured"};
    }

    // To enable actual DB operations, link against MySQL client library and implement prepared statement:
    // SELECT MAX(temp_c) FROM temperatures WHERE latitude=? AND longitude=? AND year=? AND month=? AND day=? AND grib_file=?
    // For this self-contained example without external dependencies, we return not configured.
    return {false, std::nullopt, "database_not_configured"};
}

static std::string json_error(const std::string& code) {
    return std::string("{\"error\":\"") + code + "\"}";
}

static void send_response(int client_fd, int status, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " OK\r\n";
    oss << "Content-Type: application/json; charset=utf-8\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    std::string headers = oss.str();
    send(client_fd, headers.c_str(), headers.size(), 0);
    if (!body.empty()) send(client_fd, body.c_str(), body.size(), 0);
}

static void handle_client(int client_fd) {
    char buf[8192];
    memset(buf, 0, sizeof(buf));
    ssize_t n = recv(client_fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(client_fd); return; }
    std::string req(buf, buf+n);

    // Parse request line
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) { send_response(client_fd, 400, json_error("invalid_request")); close(client_fd); return; }
    std::string line = req.substr(0, line_end);
    std::istringstream iss(line);
    std::string method, target, version;
    iss >> method >> target >> version;
    if (method != "GET") { send_response(client_fd, 405, json_error("method_not_allowed")); close(client_fd); return; }

    // Parse path and query
    std::string path = target;
    std::string query;
    size_t qm = target.find('?');
    if (qm != std::string::npos) {
        path = target.substr(0, qm);
        query = target.substr(qm+1);
    }
    if (path != ENDPOINT) { send_response(client_fd, 404, json_error("not_found")); close(client_fd); return; }

    auto params = parse_query(query);
    const char* required[] = {"latitude","longitude","year","month","day","grib_file"};
    for (const char* r: required) {
        if (params.find(r) == params.end()) {
            send_response(client_fd, 400, json_error("missing_param"));
            close(client_fd);
            return;
        }
    }

    auto lat = parse_double_in_range(params["latitude"], -90.0, 90.0);
    auto lon = parse_double_in_range(params["longitude"], -180.0, 180.0);
    auto year = parse_int_in_range(params["year"], 1900, 2100);
    auto month = parse_int_in_range(params["month"], 1, 12);
    auto day = parse_int_in_range(params["day"], 1, 31);
    std::string grib = params["grib_file"];

    if (!lat || !lon || !year || !month || !day) {
        send_response(client_fd, 400, json_error("invalid_param"));
        close(client_fd);
        return;
    }
    if (grib.size() > 64 || !std::regex_match(grib, GRIB_ALLOWED)) {
        send_response(client_fd, 400, json_error("invalid_param"));
        close(client_fd);
        return;
    }

    DbResult r = query_max_temperature(*lat, *lon, *year, *month, *day, grib);
    if (!r.success) {
        if (r.error == "database_not_configured") send_response(client_fd, 500, json_error("database_not_configured"));
        else if (r.error == "not_found") send_response(client_fd, 404, "{\"max_temperature\": null}");
        else send_response(client_fd, 500, json_error("internal_error"));
        close(client_fd);
        return;
    }

    std::ostringstream body;
    body << "{\"max_temperature\":" << *(r.max_temperature) << "}";
    send_response(client_fd, 200, body.str());
    close(client_fd);
}

static std::string http_get_local(const std::string& path_and_query) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return "{\"error\":\"internal_error\"}";
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return "{\"error\":\"internal_error\"}";
    }
    std::ostringstream req;
    req << "GET " << path_and_query << " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    std::string s = req.str();
    send(fd, s.c_str(), s.size(), 0);
    std::string resp;
    char buf[4096];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
        resp.append(buf, buf + n);
    }
    close(fd);
    // Extract body
    std::string delim = "\r\n\r\n";
    size_t pos = resp.find(delim);
    if (pos != std::string::npos) return resp.substr(pos + delim.size());
    return "{\"error\":\"internal_error\"}";
}

int main() {
    // Server thread
    std::atomic<bool> running{true};
    std::thread server([&]{
        int sfd = socket(AF_INET, SOCK_STREAM, 0);
        int opt=1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) != 0) {
            std::cerr << "Bind failed\n";
            return;
        }
        if (listen(sfd, 8) != 0) {
            std::cerr << "Listen failed\n";
            return;
        }
        while (running.load()) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(sfd, &rfds);
            timeval tv{1,0};
            int r = select(sfd+1, &rfds, nullptr, nullptr, &tv);
            if (r > 0 && FD_ISSET(sfd, &rfds)) {
                sockaddr_in cli{};
                socklen_t cl = sizeof(cli);
                int cfd = accept(sfd, (sockaddr*)&cli, &cl);
                if (cfd >= 0) {
                    handle_client(cfd);
                }
            }
        }
        close(sfd);
    });

    // Allow server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Five test cases
    std::vector<std::string> tests = {
        ENDPOINT + std::string("?latitude=40.7128&longitude=-74.0060&year=2023&month=5&day=23&grib_file=nyc_20230523.grb2"),
        ENDPOINT + std::string("?latitude=123.45&longitude=-74.0060&year=2023&month=5&day=23&grib_file=nyc_20230523.grb2"),
        ENDPOINT + std::string("?latitude=40.7128&longitude=-74.0060&year=2023&month=5&day=23&grib_file=../../etc/passwd"),
        ENDPOINT + std::string("?latitude=40.7128&longitude=-74.0060&year=2023&month=5&grib_file=nyc_20230523.grb2"),
        ENDPOINT + std::string("?latitude=-90&longitude=180&year=1900&month=12&day=31&grib_file=edge_case.grb")
    };
    for (const auto& t : tests) {
        std::cout << "Request: " << t.substr(t.find('?')+1) << "\n";
        std::cout << "Response: " << http_get_local(t) << "\n";
        std::cout << "---\n";
    }

    running.store(false);
    server.join();
    return 0;
}