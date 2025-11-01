#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

static const std::regex USERNAME_RE("^[A-Za-z0-9_]{3,32}$");
static const std::regex STOCK_RE("^[A-Za-z0-9.-]{1,16}$");
static const std::string DB_PATH = "orders_db.csv";
static const std::string HEADER = "timestamp,username,stock,quantity";

static bool ensure_db_with_header() {
    namespace fs = std::filesystem;
    try {
        bool need_header = false;
        if (!fs::exists(DB_PATH)) {
            need_header = true;
            // Create empty file first
            std::ofstream create(DB_PATH, std::ios::binary | std::ios::app);
            if (!create) return false;
            create.close();
        } else {
            auto sz = fs::file_size(DB_PATH);
            if (sz == 0) need_header = true;
        }
        if (need_header) {
            std::ofstream out(DB_PATH, std::ios::binary | std::ios::app);
            if (!out) return false;
            out << HEADER << "\n";
            out.close();
        }
        return true;
    } catch (...) {
        return false;
    }
}

static bool append_order(const std::string& username, const std::string& stock, int qty) {
    if (!ensure_db_with_header()) return false;

    // timestamp in UTC ISO8601
    std::time_t t = std::time(nullptr);
    std::tm gmt{};
#if defined(_WIN32)
    gmtime_s(&gmt, &t);
#else
    gmtime_r(&t, &gmt);
#endif
    std::ostringstream ts;
    ts << std::put_time(&gmt, "%Y-%m-%dT%H:%M:%SZ");

    std::ofstream out(DB_PATH, std::ios::binary | std::ios::app);
    if (!out) return false;
    out << ts.str() << "," << username << "," << stock << "," << qty << "\n";
    out.close();
    return true;
}

std::string process_buy_order(const std::string& username, const std::string& stock_name, int stock_quantity) {
    try {
        if (!std::regex_match(username, USERNAME_RE)) return {};
        if (!std::regex_match(stock_name, STOCK_RE)) return {};
        if (stock_quantity < 1 || stock_quantity > 1000000) return {};

        std::string stock_norm = stock_name;
        for (char& c : stock_norm) c = (char)std::toupper((unsigned char)c);

        if (!append_order(username, stock_norm, stock_quantity)) return {};
        return "/users/" + username + "/profile";
    } catch (...) {
        return {};
    }
}

static void run_test(const std::string& user, const std::string& stock, int qty) {
    std::string res = process_buy_order(user, stock, qty);
    std::cout << "Input: user=" << user << ", stock=" << stock << ", qty=" << qty << "\n";
    std::cout << "Result: " << (res.empty() ? "ERROR" : res) << "\n----\n";
}

int main() {
    // 5 test cases
    run_test("alice_01", "AAPL", 10);
    run_test("bob", "GOOG", 100);
    run_test("charlie", "msft", 5000);
    run_test("bad name", "TSLA", 10); // invalid username
    run_test("dave", "BRK.B", 0);     // invalid quantity
    return 0;
}