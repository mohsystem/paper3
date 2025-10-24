#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <random>
#include <cctype>

static const std::string DB_PATH_CPP = "users_db_cpp.jsonl";

static bool is_valid_name(const std::string& name_in) {
    std::string name;
    // trim
    size_t start = name_in.find_first_not_of(" \t\r\n");
    size_t end = name_in.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return false;
    name = name_in.substr(start, end - start + 1);

    if (name.size() < 2 || name.size() > 100) return false;
    if (!std::isalpha(static_cast<unsigned char>(name.front()))) return false;
    if (!std::isalpha(static_cast<unsigned char>(name.back()))) return false;
    for (char c : name) {
        if (std::isalpha(static_cast<unsigned char>(c)) || c==' ' || c=='-' || c=='\'') continue;
        return false;
    }
    return true;
}

static bool is_valid_email(const std::string& email_in) {
    // lower and trim
    size_t start = email_in.find_first_not_of(" \t\r\n");
    size_t end = email_in.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return false;
    std::string email = email_in.substr(start, end - start + 1);
    if (email.size() < 3 || email.size() > 254) return false;

    // convert to lower
    for (char& c : email) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    size_t at_pos = email.find('@');
    if (at_pos == std::string::npos) return false;
    if (email.find('@', at_pos + 1) != std::string::npos) return false; // multiple '@'

    std::string local = email.substr(0, at_pos);
    std::string domain = email.substr(at_pos + 1);
    if (local.empty() || domain.size() < 3) return false;
    if (domain.front() == '.' || domain.back() == '.') return false;
    if (domain.find('.') == std::string::npos) return false;

    auto valid_local_char = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c=='.' || c=='_' || c=='%' || c=='+' || c=='-';
    };
    auto valid_domain_char = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c=='.' || c=='-';
    };

    if (local.front()=='.' || local.back()=='.') return false;
    if (local.find("..") != std::string::npos) return false;
    for (char c : local) if (!valid_local_char(c)) return false;

    if (domain.find("..") != std::string::npos) return false;
    for (char c : domain) if (!valid_domain_char(c)) return false;

    size_t last_dot = domain.rfind('.');
    if (last_dot == std::string::npos) return false;
    if (domain.size() - last_dot - 1 < 2) return false; // TLD >=2

    return true;
}

static bool is_valid_age(int age) {
    return age >= 0 && age <= 150;
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, b - a + 1);
}

static std::string escape_json(const std::string& s) {
    std::ostringstream oss;
    for (unsigned char c : s) {
        switch (c) {
            case '\\': oss << "\\\\"; break;
            case '"':  oss << "\\\""; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    oss << buf;
                } else {
                    oss << static_cast<char>(c);
                }
        }
    }
    return oss.str();
}

static std::string generate_id() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    uint64_t r = gen();
    std::ostringstream oss;
    oss << std::hex;
    for (int i = 7; i >= 0; --i) {
        unsigned int byte = (r >> (i*8)) & 0xFFu;
        if (byte < 16) oss << '0';
        oss << byte;
    }
    return oss.str();
}

static bool append_line_to_db(const std::string& line) {
    std::ofstream ofs(DB_PATH_CPP, std::ios::out | std::ios::app | std::ios::binary);
    if (!ofs.good()) return false;
    ofs << line;
    ofs.flush();
    return ofs.good();
}

static bool create_user(const std::string& name_in, const std::string& email_in, int age, std::string& out_message) {
    std::string n = trim(name_in);
    std::string e = trim(email_in);

    if (!is_valid_name(n)) {
        out_message = "ERROR: invalid name";
        return false;
    }
    if (!is_valid_email(e)) {
        out_message = "ERROR: invalid email";
        return false;
    }
    if (!is_valid_age(age)) {
        out_message = "ERROR: invalid age";
        return false;
    }

    std::time_t t = std::time(nullptr);
    char tsbuf[32];
    std::strftime(tsbuf, sizeof(tsbuf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));

    std::string id = generate_id();
    std::ostringstream line;
    line << "{\"id\":\"" << escape_json(id) << "\",\"name\":\"" << escape_json(n)
         << "\",\"email\":\"" << escape_json(e) << "\",\"age\":" << age
         << ",\"ts\":\"" << tsbuf << "\"}\n";

    if (!append_line_to_db(line.str())) {
        out_message = "ERROR: storage failure";
        return false;
    }
    out_message = std::string("OK: ") + id;
    return true;
}

int main() {
    std::string msg;

    create_user("John Doe", "john.doe@example.com", 30, msg);
    std::cout << msg << std::endl;

    create_user("A", "not-an-email", 20, msg);
    std::cout << msg << std::endl;

    create_user("Mary-Jane O'Neil", "mary@example", -1, msg);
    std::cout << msg << std::endl;

    create_user("  Robert   ", "robert.smith@example.co.uk", 44, msg);
    std::cout << msg << std::endl;

    create_user("ThisNameIsWayTooLong_ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "long.name@example.com", 20, msg);
    std::cout << msg << std::endl;

    return 0;
}