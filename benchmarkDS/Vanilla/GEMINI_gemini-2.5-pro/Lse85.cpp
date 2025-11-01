#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <optional>

// Simulating a database table called 'Sensor' in memory
class SensorDB {
private:
    static std::map<std::string, double> database;

public:
    static void initialize() {
        // Key is "latitude:longitude:date"
        database["34.0522:-118.2437:2023-10-26"] = 25.5;
        database["40.7128:-74.0060:2023-10-26"] = 15.2;
        database["48.8566:2.3522:2023-10-27"] = 12.8;
        database["35.6895:139.6917:2023-10-26"] = 18.0;
        database["-33.8688:151.2093:2023-10-28"] = 22.3;
    }

    static std::map<std::string, double>& getDB() {
        return database;
    }
};

std::map<std::string, double> SensorDB::database;


/**
 * This function simulates an API endpoint call.
 * It connects to the 'database' and gets the temperature for the given location and date.
 *
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date in "YYYY-MM-DD" format.
 * @return An optional containing the temperature if found, otherwise an empty optional.
 */
std::optional<double> temperature_for_location(double latitude, double longitude, const std::string& date) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << latitude << ":" << longitude << ":" << date;
    std::string key = ss.str();

    auto& db = SensorDB::getDB();
    auto it = db.find(key);

    if (it != db.end()) {
        return it->second;
    } else {
        return std::nullopt;
    }
}

int main() {
    SensorDB::initialize();

    // --- 5 Test Cases ---

    // Test Case 1: Existing data
    std::cout << "Test Case 1: (LA, 2023-10-26)" << std::endl;
    auto temp1 = temperature_for_location(34.0522, -118.2437, "2023-10-26");
    if (temp1.has_value()) {
        std::cout << "  -> Temperature: " << temp1.value() << std::endl;
    } else {
        std::cout << "  -> Temperature not found." << std::endl;
    }

    // Test Case 2: Existing data
    std::cout << "\nTest Case 2: (NYC, 2023-10-26)" << std::endl;
    auto temp2 = temperature_for_location(40.7128, -74.0060, "2023-10-26");
    if (temp2) { // .has_value() is implicit in boolean context
        std::cout << "  -> Temperature: " << *temp2 << std::endl;
    } else {
        std::cout << "  -> Temperature not found." << std::endl;
    }

    // Test Case 3: Existing data
    std::cout << "\nTest Case 3: (Paris, 2023-10-27)" << std::endl;
    auto temp3 = temperature_for_location(48.8566, 2.3522, "2023-10-27");
    if (temp3) {
        std::cout << "  -> Temperature: " << *temp3 << std::endl;
    } else {
        std::cout << "  -> Temperature not found." << std::endl;
    }
    
    // Test Case 4: Non-existing date
    std::cout << "\nTest Case 4: (Tokyo, Non-existing date)" << std::endl;
    auto temp4 = temperature_for_location(35.6895, 139.6917, "2023-10-27");
    if (temp4) {
        std::cout << "  -> Temperature: " << *temp4 << std::endl;
    } else {
        std::cout << "  -> Temperature not found." << std::endl;
    }

    // Test Case 5: Non-existing location
    std::cout << "\nTest Case 5: (Non-existing location)" << std::endl;
    auto temp5 = temperature_for_location(0.0, 0.0, "2023-10-26");
    if (temp5) {
        std::cout << "  -> Temperature: " << *temp5 << std::endl;
    } else {
        std::cout << "  -> Temperature not found." << std::endl;
    }

    return 0;
}