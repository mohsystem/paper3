#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

// Requires MySQL C client library (libmysqlclient)
#include <mysql/mysql.h>

static bool is_valid_grib_file(const std::string& s) {
    static const std::regex re("^[A-Za-z0-9._-]{1,128}$");
    return std::regex_match(s, re);
}

static bool parse_double(const std::string& s, double& out) {
    char* end = nullptr;
    errno = 0;
    double v = strtod(s.c_str(), &end);
    if (errno != 0 || end == s.c_str() || *end != '\0' || std::isnan(v) || std::isinf(v)) return false;
    out = v;
    return true;
}

static bool parse_int(const std::string& s, int& out) {
    char* end = nullptr;
    errno = 0;
    long v = strtol(s.c_str(), &end, 10);
    if (errno != 0 || end == s.c_str() || *end != '\0' || v < INT32_MIN || v > INT32_MAX) return false;
    out = static_cast<int>(v);
    return true;
}

static std::map<std::string, std::string> parse_query(const std::string& query) {
    std::map<std::string, std::string> m;
    std::string key, val;
    std::istringstream ss(query);
    std::string token;
    while (std::getline(ss, token, '&')) {
        auto pos = token.find('=');
        if (pos == std::string::npos) {
            m[token] = "";
        } else {
            m[token.substr(0, pos)] = token.substr(pos + 1);
        }
    }
    return m;
}

static std::string json_escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"' || c == '\\') { out.push_back('\\'); out.push_back(c); }
        else if (c == '\n') { out += "\\n"; }
        else { out.push_back(c); }
    }
    return out;
}

static bool mysql_query_max_temp(double latitude, double longitude, int year, int month, int day, const std::string& grib_file, double& out, std::string& err) {
    if (!is_valid_grib_file(grib_file)) { err = "invalid grib_file"; return false; }
    const char* host = getenv("DB_HOST"); if (!host) host = "127.0.0.1";
    const char* user = getenv("DB_USER"); if (!user) user = "root";
    const char* pass = getenv("DB_PASS"); if (!pass) pass = "";
    const char* db   = getenv("DB_NAME"); if (!db) db   = "weather";
    unsigned int port = 3306;
    if (const char* p = getenv("DB_PORT")) port = static_cast<unsigned int>(strtoul(p, nullptr, 10));

    MYSQL* conn = mysql_init(nullptr);
    if (!conn) { err = "mysql_init failed"; return false; }
    unsigned int timeout = 10;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    if (!mysql_real_connect(conn, host, user, pass, db, port, nullptr, 0)) {
        err = mysql_error(conn);
        mysql_close(conn);
        return false;
    }

    // Build safe query (numbers + strictly validated grib_file)
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "SELECT MAX(temperature) AS max_temp FROM temperatures "
             "WHERE latitude=%.10f AND longitude=%.10f AND year=%d AND month=%d AND day=%d AND grib_file='%s'",
             latitude, longitude, year, month, day, grib_file.c_str());

    if (mysql_query(conn, buf) != 0) {
        err = mysql_error(conn);
        mysql_close(conn);
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        err = mysql_error(conn);
        mysql_close(conn);
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    bool ok = true;
    if (!row || !row[0]) {
        ok = false; // NULL
        err.clear();
    } else {
        out = atof(row[0]);
    }
    mysql_free_result(res);
    mysql_close(conn);
    return ok;
}

static std::string handle_temperature_request(const std::map<std::string, std::string>& q) {
    double lat, lon;
    int year, month, day;
    std::string grib;
    try {
        auto it = q.find("latitude"); if (it == q.end() || !parse_double(it->second, lat) || !(lat >= -90.0 && lat <= 90.0)) throw std::runtime_error("invalid latitude");
        it = q.find("longitude"); if (it == q.end() || !parse_double(it->second, lon) || !(lon >= -180.0 && lon <= 180.0)) throw std::runtime_error("invalid longitude");
        it = q.find("year"); if (it == q.end() || !parse_int(it->second, year) || !(year >= 1900 && year <= 2100)) throw std::runtime_error("invalid year");
        it = q.find("month"); if (it == q.end() || !parse_int(it->second, month) || !(month >= 1 && month <= 12)) throw std::runtime_error("invalid month");
        it = q.find("day"); if (it == q.end() || !parse_int(it->second, day) || !(day >= 1 && day <= 31)) throw std::runtime_error("invalid day");
        it = q.find("grib_file"); if (it == q.end()) throw std::runtime_error("missing grib_file");
        grib = it->second;
        if (!is_valid_grib_file(grib)) throw std::runtime_error("invalid grib_file");
    } catch (const std::exception& e) {
        return std::string("{\"error\":\"") + json_escape(e.what()) + "\"}";
    }

    double maxTemp = NAN;
    std::string err;
    bool has = mysql_query_max_temp(lat, lon, year, month, day, grib, maxTemp, err);
    if (!err.empty()) {
        return std::string("{\"error\":\"") + json_escape(err) + "\"}";
    }
    if (!has) {
        return "{\"max_temperature\":null}";
    }
    std::ostringstream oss;
    oss << "{\"max_temperature\":" << maxTemp << "}";
    return oss.str();
}

static void http_server_run(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return; }
    if (listen(fd, 5) != 0) { close(fd); return; }

    while (true) {
        int cfd = accept(fd, nullptr, nullptr);
        if (cfd < 0) continue;
        char buf[4096];
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(cfd); continue; }
        buf[n] = '\0';
        // Parse request line
        std::istringstream req(std::string(buf));
        std::string method, path, protocol;
        req >> method >> path >> protocol;
        std::string body;
        int status = 200;
        std::string response;

        if (method != "GET") {
            status = 405;
            response = "{\"error\":\"Method Not Allowed\"}";
        } else {
            // path like /api/temperature_for_location?query
            std::string endpoint = "/api/temperature_for_location";
            std::string::size_type qpos = path.find('?');
            std::string pth = (qpos == std::string::npos) ? path : path.substr(0, qpos);
            std::string qstr = (qpos == std::string::npos) ? "" : path.substr(qpos + 1);
            if (pth != endpoint) {
                status = 404;
                response = "{\"error\":\"Not Found\"}";
            } else {
                auto qmap = parse_query(qstr);
                response = handle_temperature_request(qmap);
            }
        }
        std::ostringstream hdr;
        hdr << "HTTP/1.1 " << status << " OK\r\n"
            << "Content-Type: application/json; charset=utf-8\r\n"
            << "Content-Length: " << response.size() << "\r\n"
            << "Connection: close\r\n\r\n";
        std::string header = hdr.str();
        send(cfd, header.c_str(), header.size(), 0);
        send(cfd, response.c_str(), response.size(), 0);
        close(cfd);
    }
    close(fd);
}

int main() {
    // Start HTTP server in background
    uint16_t port = 8082;
    std::thread srv([port]() { http_server_run(port); });
    srv.detach();

    // 5 test cases invoking the core function directly (may fail if DB not configured)
    struct Case { double lat, lon; int y, m, d; const char* g; };
    Case tests[5] = {
        {40.7128, -74.0060, 2023, 7, 15, "sample1.grb"},
        {34.0522, -118.2437, 2022, 12, 1, "sample2.grb"},
        {-33.8688, 151.2093, 2021, 1, 31, "sample3.grb"},
        {51.5074, -0.1278, 2020, 6, 21, "sample4.grb"},
        {35.6895, 139.6917, 2019, 3, 10, "sample5.grb"}
    };
    for (auto& t : tests) {
        std::map<std::string, std::string> q {
            {"latitude", std::to_string(t.lat)},
            {"longitude", std::to_string(t.lon)},
            {"year", std::to_string(t.y)},
            {"month", std::to_string(t.m)},
            {"day", std::to_string(t.d)},
            {"grib_file", t.g}
        };
        std::cout << handle_temperature_request(q) << std::endl;
    }
    return 0;
}