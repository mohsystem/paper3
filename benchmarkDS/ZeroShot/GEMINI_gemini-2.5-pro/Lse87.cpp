#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <iomanip>
#include <sstream>
#include <regex>

// This class simulates the web application context.
class Lse87 {
private:
    // This map simulates a database table 'temperatures'.
    // In a real C++ application, you would use a database library like
    // libpqxx (for PostgreSQL) or OTL.
    static std::map<std::string, double> temperatureDatabase;

public:
    static void initializeDatabase() {
        // Populating our mock database with some data.
        temperatureDatabase["40.7128,-74.0060,2023-10-27"] = 15.5;
        temperatureDatabase["34.0522,-118.2437,2023-10-27"] = 22.1;
        temperatureDatabase["48.8566,2.3522,2023-10-26"] = 12.8;
    }

    /**
     * Securely retrieves temperature for a given location and date.
     * This function simulates a web API endpoint handler.
     *
     * @param latitude The latitude of the location (-90 to 90).
     * @param longitude The longitude of the location (-180 to 180).
     * @param dateStr The date in 'YYYY-MM-DD' format.
     * @return An optional containing the temperature if found, otherwise an empty optional.
     */
    static std::optional<double> temperatureForLocation(double latitude, double longitude, const std::string& dateStr) {
        // 1. **Input Validation**: Crucial security step to prevent invalid data processing.
        if (latitude < -90.0 || latitude > 90.0) {
            std::cerr << "Error: Invalid latitude value." << std::endl;
            return std::nullopt;
        }
        if (longitude < -180.0 || longitude > 180.0) {
            std::cerr << "Error: Invalid longitude value." << std::endl;
            return std::nullopt;
        }
        // Basic regex for YYYY-MM-DD format validation
        const std::regex date_pattern(R"(\d{4}-\d{2}-\d{2})");
        if (!std::regex_match(dateStr, date_pattern)) {
             std::cerr << "Error: Invalid date format. Please use YYYY-MM-DD." << std::endl;
             return std::nullopt;
        }

        // 2. **Secure Database Query (Demonstration)**
        // In a real application, ALWAYS use prepared statements or parameterized queries.
        // DO NOT build queries by concatenating strings.
        /*
        // --- START OF SECURE libpqxx EXAMPLE (for illustration) ---
        // try {
        //     pqxx::connection C("dbname=mydb user=myuser password=mypass hostaddr=127.0.0.1 port=5432");
        //     pqxx::work W(C);
        //
        //     std::string secureQuery = "SELECT temperature FROM temperatures WHERE latitude = $1 AND longitude = $2 AND date = $3;";
        //
        //     // The library prepares the statement and handles escaping of parameters.
        //     pqxx::result R = W.exec_params(secureQuery, latitude, longitude, dateStr);
        //     W.commit();
        //
        //     if (!R.empty()) {
        //         return R[0][0].as<double>();
        //     } else {
        //         return std::nullopt;
        //     }
        // } catch (const std::exception &e) {
        //     std::cerr << "Database error: " << e.what() << std::endl;
        //     return std::nullopt;
        // }
        // --- END OF SECURE libpqxx EXAMPLE ---
        */

        // **Simulating the secure database lookup using our map**
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << latitude << "," << longitude << "," << dateStr;
        std::string dbKey = ss.str();

        auto it = temperatureDatabase.find(dbKey);
        if (it != temperatureDatabase.end()) {
            return it->second;
        }
        
        return std::nullopt;
    }
};

// Initialize the static database
std::map<std::string, double> Lse87::temperatureDatabase;

int main() {
    Lse87::initializeDatabase();
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Valid request, data exists
    std::cout << "Test Case 1 (Success):" << std::endl;
    if (auto temp = Lse87::temperatureForLocation(40.7128, -74.0060, "2023-10-27")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Temperature not found." << std::endl;
    }

    // Test Case 2: Valid request, data does not exist
    std::cout << "\nTest Case 2 (Not Found):" << std::endl;
    if (auto temp = Lse87::temperatureForLocation(10.0, 20.0, "2023-01-01")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Temperature not found." << std::endl;
    }

    // Test Case 3: Invalid latitude
    std::cout << "\nTest Case 3 (Invalid Latitude):" << std::endl;
    if (auto temp = Lse87::temperatureForLocation(100.0, -74.0060, "2023-10-27")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Request failed as expected." << std::endl;
    }

    // Test Case 4: Invalid longitude
    std::cout << "\nTest Case 4 (Invalid Longitude):" << std::endl;
    if (auto temp = Lse87::temperatureForLocation(40.7128, -200.0, "2023-10-27")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Request failed as expected." << std::endl;
    }

    // Test Case 5: Invalid date format
    std::cout << "\nTest Case 5 (Invalid Date):" << std::endl;
    if (auto temp = Lse87::temperatureForLocation(34.0522, -118.2437, "27-10-2023")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Request failed as expected." << std::endl;
    }

    return 0;
}