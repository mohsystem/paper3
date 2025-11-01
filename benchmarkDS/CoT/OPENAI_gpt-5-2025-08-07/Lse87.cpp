#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <cmath>

class SecureDB {
public:
    void insert(double lat, double lon, const std::string& dateISO, double temperature) {
        std::lock_guard<std::mutex> lock(mu_);
        data_[key(lat, lon, dateISO)] = temperature;
    }

    bool getTemperature(double lat, double lon, const std::string& dateISO, double& outTemp) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = data_.find(key(lat, lon, dateISO));
        if (it == data_.end()) return false;
        outTemp = it->second;
        return true;
    }

private:
    static std::string key(double lat, double lon, const std::string& dateISO) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << lat << "|" << lon << "|" << dateISO;
        return oss.str();
    }

    std::unordered_map<std::string, double> data_;
    std::mutex mu_;
};

static SecureDB& getDB() {
    static SecureDB db;
    return db;
}

static bool isValidLat(double lat) {
    return std::isfinite(lat) && lat >= -90.0 && lat <= 90.0;
}

static bool isValidLon(double lon) {
    return std::isfinite(lon) && lon >= -180.0 && lon <= 180.0;
}

static bool isValidISODate(const std::string& s) {
    if (s.size() != 10) return false;
    if (s[4] != '-' || s[7] != '-') return false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (s[i] < '0' || s[i] > '9') return false;
    }
    int y = std::stoi(s.substr(0,4));
    int m = std::stoi(s.substr(5,2));
    int d = std::stoi(s.substr(8,2));
    if (m < 1 || m > 12) return false;
    if (d < 1 || d > 31) return false;
    (void)y;
    return true;
}

// Function: accepts inputs as parameters and returns the temperature; returns NAN if not found or invalid input
double temperature_for_location(double latitude, double longitude, const std::string& dateISO) {
    if (!isValidLat(latitude) || !isValidLon(longitude)) return std::numeric_limits<double>::quiet_NaN();
    if (!isValidISODate(dateISO)) return std::numeric_limits<double>::quiet_NaN();
    double temp = 0.0;
    if (getDB().getTemperature(latitude, longitude, dateISO, temp)) {
        return temp;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

static void seedSampleData() {
    getDB().insert(37.7749, -122.4194, "2023-10-01", 20.5);
    getDB().insert(40.7128, -74.0060, "2023-10-01", 18.1);
    getDB().insert(51.5074, -0.1278, "2023-12-25", 5.2);
    getDB().insert(-33.8688, 151.2093, "2024-01-15", 27.3);
    getDB().insert(35.6895, 139.6917, "2023-07-20", 30.4);
}

int main() {
    seedSampleData();

    // 5 test cases
    double t1 = temperature_for_location(37.7749, -122.4194, "2023-10-01");
    double t2 = temperature_for_location(35.6895, 139.6917, "2023-07-20");
    double t3 = temperature_for_location(37.7749, -122.4194, "2023-10-02");
    double t4 = temperature_for_location(100.0, -122.4194, "2023-10-01");
    double t5 = temperature_for_location(40.7128, -74.0060, "2023-13-01");

    std::cout << "Test1: " << t1 << "\n";
    std::cout << "Test2: " << t2 << "\n";
    std::cout << "Test3: " << t3 << "\n";
    std::cout << "Test4: " << t4 << "\n";
    std::cout << "Test5: " << t5 << "\n";

    return 0;
}