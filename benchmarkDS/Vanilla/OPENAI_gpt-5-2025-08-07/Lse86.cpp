// Note: This single-file example uses POSIX sockets and MySQL C API (libmysqlclient).
// It may require platform-specific adjustments and linking with -lmysqlclient.
// A minimal HTTP server is implemented to handle GET /api/temperature_for_location requests.

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cctype>

#ifdef _WIN32
// For Windows, Winsock initialization would be needed; omitted for brevity.
#pragma message("This example is POSIX-oriented; Windows socket setup not included.")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <mysql/mysql.h>

static std::string getenv_str(const char* k, const char* defv) {
    const char* v = std::getenv(k);
    return v ? std::string(v) : std::string(defv);
}

static unsigned int getenv_uint(const char* k, unsigned int defv) {
    const char* v = std::getenv(k);
    if (!v) return defv;
    try { return static_cast<unsigned int>(std::stoul(v)); } catch (...) { return defv; }
}

static std::map<std::string, std::string> parse_query(const std::string& qs) {
    std::map<std::string, std::string> m;
    std::string key, val;
    std::istringstream ss(qs);
    std::string pair;
    auto url_decode = [](const std::string& s)->std::string{
        std::string out; out.reserve(s.size());
        for (size_t i=0;i<s.size();++i) {
            if (s[i]=='%') {
                if (i+2 < s.size()) {
                    std::string hex = s.substr(i+1,2);
                    char c = (char) strtol(hex.c_str(), nullptr, 16);
                    out.push_back(c);
                    i+=2;
                }
            } else if (s[i]=='+') {
                out.push_back(' ');
            } else {
                out.push_back(s[i]);
            }
        }
        return out;
    };
    while (std::getline(ss, pair, '&')) {
        size_t eq = pair.find('=');
        if (eq == std::string::npos) {
            m[url_decode(pair)] = "";
        } else {
            m[url_decode(pair.substr(0,eq))] = url_decode(pair.substr(eq+1));
        }
    }
    return m;
}

static std::string json_escape(const std::string& s) {
    std::string out; out.reserve(s.size()+4);
    out.push_back('"');
    for (char c : s) {
        if (c == '\\' || c=='"') { out.push_back('\\'); out.push_back(c); }
        else if (c == '\n') { out += "\\n"; }
        else if (c == '\r') { out += "\\r"; }
        else if (c == '\t') { out += "\\t"; }
        else { out.push_back(c); }
    }
    out.push_back('"');
    return out;
}

// Function: accepts parameters and returns max temperature (or NaN if none). errOut holds error message if any.
static double queryMaxTemperature(
    double latitude,
    double longitude,
    int year,
    int month,
    int day,
    const std::string& gribFile,
    const std::string& host,
    unsigned int port,
    const std::string& user,
    const std::string& pass,
    const std::string& db,
    std::string& errOut
) {
    errOut.clear();
    MYSQL *conn = mysql_init(nullptr);
    if (!conn) { errOut = "mysql_init failed"; return NAN; }
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), port, nullptr, 0)) {
        errOut = mysql_error(conn);
        mysql_close(conn);
        return NAN;
    }
    const char* sql = "SELECT MAX(temperature) AS max_temp FROM temperatures "
                      "WHERE latitude=? AND longitude=? AND year=? AND month=? AND day=? AND grib_file=?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) { errOut = "mysql_stmt_init failed"; mysql_close(conn); return NAN; }
    if (mysql_stmt_prepare(stmt, sql, (unsigned long)std::strlen(sql)) != 0) {
        errOut = mysql_stmt_error(stmt); mysql_stmt_close(stmt); mysql_close(conn); return NAN;
    }

    MYSQL_BIND bind[6];
    std::memset(bind, 0, sizeof(bind));
    double lat = latitude; double lon = longitude;
    int y = year, m = month, d = day;
    unsigned long gribLen = (unsigned long)gribFile.size();

    bind[0].buffer_type = MYSQL_TYPE_DOUBLE; bind[0].buffer = (void*)&lat;
    bind[1].buffer_type = MYSQL_TYPE_DOUBLE; bind[1].buffer = (void*)&lon;
    bind[2].buffer_type = MYSQL_TYPE_LONG;   bind[2].buffer = (void*)&y;
    bind[3].buffer_type = MYSQL_TYPE_LONG;   bind[3].buffer = (void*)&m;
    bind[4].buffer_type = MYSQL_TYPE_LONG;   bind[4].buffer = (void*)&d;
    bind[5].buffer_type = MYSQL_TYPE_STRING; bind[5].buffer = (void*)gribFile.c_str(); bind[5].buffer_length = gribLen; bind[5].length = &gribLen;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        errOut = mysql_stmt_error(stmt); mysql_stmt_close(stmt); mysql_close(conn); return NAN;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        errOut = mysql_stmt_error(stmt); mysql_stmt_close(stmt); mysql_close(conn); return NAN;
    }

    double max_temp = 0.0;
    my_bool is_null = 0;
    MYSQL_BIND outb[1];
    std::memset(outb, 0, sizeof(outb));
    outb[0].buffer_type = MYSQL_TYPE_DOUBLE;
    outb[0].buffer = (void*)&max_temp;
    outb[0].is_null = &is_null;

    if (mysql_stmt_bind_result(stmt, outb) != 0) {
        errOut = mysql_stmt_error(stmt); mysql_stmt_close(stmt); mysql_close(conn); return NAN;
    }
    double result = NAN;
    if (mysql_stmt_fetch(stmt) == 0) {
        if (!is_null) result = max_temp;
    } else {
        // no rows or error; keep NAN
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    return result;
}

#ifndef _WIN32
static void handle_client(int fd) {
    char buf[4096];
    int n = recv(fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(fd); return; }
    buf[n] = '\0';
    std::string req(buf);

    // parse first line
    size_t p1 = req.find(' ');
    if (p1 == std::string::npos) { close(fd); return; }
    size_t p2 = req.find(' ', p1+1);
    if (p2 == std::string::npos) { close(fd); return; }
    std::string method = req.substr(0, p1);
    std::string target = req.substr(p1+1, p2 - (p1+1));

    std::string path = target;
    std::string query;
    size_t qpos = target.find('?');
    if (qpos != std::string::npos) {
        path = target.substr(0, qpos);
        query = target.substr(qpos+1);
    }

    if (method != "GET" || path != "/api/temperature_for_location") {
        std::string body = "{\"error\":\"Not Found\"}";
        std::ostringstream resp;
        resp << "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\nConnection: close\r\n\r\n" << body;
        send(fd, resp.str().c_str(), (int)resp.str().size(), 0);
        close(fd);
        return;
    }

    auto params = parse_query(query);
    std::vector<std::string> required = {"latitude","longitude","year","month","day","grib_file"};
    for (auto &r : required) {
        if (params.find(r) == params.end() || params[r].empty()) {
            std::string body = std::string("{\"error\":\"Missing parameter: ") + r + "\"}";
            std::ostringstream resp;
            resp << "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\nConnection: close\r\n\r\n" << body;
            send(fd, resp.str().c_str(), (int)resp.str().size(), 0);
            close(fd);
            return;
        }
    }

    try {
        double lat = std::stod(params["latitude"]);
        double lon = std::stod(params["longitude"]);
        int year = std::stoi(params["year"]);
        int month = std::stoi(params["month"]);
        int day = std::stoi(params["day"]);
        std::string grib = params["grib_file"];

        std::string host = getenv_str("DB_HOST", "127.0.0.1");
        unsigned int port = getenv_uint("DB_PORT", 3306);
        std::string user = getenv_str("DB_USER", "root");
        std::string pass = getenv_str("DB_PASS", "");
        std::string db   = getenv_str("DB_NAME", "weather");

        std::string err;
        double maxT = queryMaxTemperature(lat, lon, year, month, day, grib, host, port, user, pass, db, err);
        if (!err.empty()) {
            std::string body = std::string("{\"error\":") + json_escape(err) + "}";
            std::ostringstream resp;
            resp << "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\nConnection: close\r\n\r\n" << body;
            send(fd, resp.str().c_str(), (int)resp.str().size(), 0);
            close(fd);
            return;
        }
        std::ostringstream body;
        body << "{"
             << "\"latitude\":" << lat << ","
             << "\"longitude\":" << lon << ","
             << "\"year\":" << year << ","
             << "\"month\":" << month << ","
             << "\"day\":" << day << ","
             << "\"grib_file\":" << json_escape(grib) << ","
             << "\"max_temperature\":" << (std::isnan(maxT) ? std::string("null") : (static_cast<std::ostringstream&&>(std::ostringstream() << maxT).str()))
             << "}";

        std::string bodyStr = body.str();
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << bodyStr.size() << "\r\nConnection: close\r\n\r\n" << bodyStr;
        std::string respStr = resp.str();
        send(fd, respStr.c_str(), (int)respStr.size(), 0);
    } catch (...) {
        std::string body = "{\"error\":\"Invalid parameter format\"}";
        std::ostringstream resp;
        resp << "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\nConnection: close\r\n\r\n" << body;
        send(fd, resp.str().c_str(), (int)resp.str().size(), 0);
    }
    close(fd);
}

static void server_loop(int port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return; }
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_ANY); addr.sin_port = htons(port);
    if (bind(s, (sockaddr*)&addr, sizeof(addr)) != 0) { perror("bind"); close(s); return; }
    if (listen(s, 16) != 0) { perror("listen"); close(s); return; }

    while (true) {
        int c = accept(s, nullptr, nullptr);
        if (c < 0) { continue; }
        std::thread(handle_client, c).detach();
    }
}

int main() {
    int port = (int)getenv_uint("CPP_PORT", 8082);
    std::thread t(server_loop, port);
    t.detach();

    // 5 test cases by calling the function directly (demonstration)
    std::string host = getenv_str("DB_HOST", "127.0.0.1");
    unsigned int dbport = getenv_uint("DB_PORT", 3306);
    std::string user = getenv_str("DB_USER", "root");
    std::string pass = getenv_str("DB_PASS", "");
    std::string db   = getenv_str("DB_NAME", "weather");

    struct T { double lat, lon; int y,m,d; const char* gf; };
    std::vector<T> tests = {
        {40.7128, -74.0060, 2024,1,15, "gfs_20240115.grb2"},
        {34.0522, -118.2437, 2023,7,4, "era5_20230704.grib"},
        {51.5074, -0.1278, 2022,12,25, "ukmet_20221225.grb"},
        {35.6895, 139.6917, 2021,8,1, "jma_20210801.grb2"},
        {-33.8688, 151.2093, 2020,2,29, "bom_20200229.grib"},
    };
    for (auto &tc : tests) {
        std::string err;
        double val = queryMaxTemperature(tc.lat, tc.lon, tc.y, tc.m, tc.d, tc.gf, host, dbport, user, pass, db, err);
        if (!err.empty()) {
            std::cout << "C++ test error: " << err << std::endl;
        } else {
            std::cout << "C++ test result for " << tc.gf << ": " << (std::isnan(val) ? std::string("null") : (static_cast<std::ostringstream&&>(std::ostringstream() << val).str())) << std::endl;
        }
    }

    // Keep process alive for manual HTTP tests
    std::this_thread::sleep_for(std::chrono::hours(24));
    return 0;
}
#else
int main() {
    // Windows placeholder
    std::cout << "C++ server example is POSIX-oriented; not running on Windows in this snippet." << std::endl;
    return 0;
}
#endif