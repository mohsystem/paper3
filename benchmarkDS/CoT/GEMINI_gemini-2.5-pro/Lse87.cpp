#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <optional>

// C++ has no standard database API. This example simulates the secure pattern
// of using prepared statements, which is crucial for preventing SQL injection.
// Libraries like SOCI, OTL, or specific vendor connectors (e.g., mysql-connector-c++)
// provide this functionality.

class MockDatabase {
private:
    // Using a map with a tuple as a key to simulate a database table
    std::map<std::tuple<double, double, std::string>, double> temperatures;

public:
    MockDatabase() {
        // Populate with some data
        temperatures[std::make_tuple(40.71, -74.00, "2023-10-26")] = 15.5;
        temperatures[std::make_tuple(34.05, -118.24, "2023-10-26")] = 22.0;
        temperatures[std::make_tuple(48.85, 2.35, "2023-10-27")] = 12.3;
        temperatures[std::make_tuple(35.68, 139.69, "2023-10-27")] = 18.1;
    }

    std::optional<double> query(double lat, double lon, const std::string& date) {
        auto it = temperatures.find(std::make_tuple(lat, lon, date));
        if (it != temperatures.end()) {
            return it->second;
        }
        return std::nullopt;
    }
};

/**
 * Simulates connecting to a database and securely querying for the temperature.
 *
 * @param db A reference to our mock database.
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date for the temperature reading (e.g., "YYYY-MM-DD").
 * @return An optional<double> containing the temperature, or nullopt if not found.
 */
std::optional<double> temperature_for_location(MockDatabase& db, double latitude, double longitude, const std::string& date) {
    // The SQL query with '?' placeholders to prevent SQL injection.
    const std::string sql = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND aqi_date = ?";

    std::cout << "\n--- C++ Secure Pattern Demonstration ---" << std::endl;
    std::cout << "Preparing statement: " << sql << std::endl;
    std::cout << "Binding parameter 1 (latitude): " << latitude << std::endl;
    std::cout << "Binding parameter 2 (longitude): " << longitude << std::endl;
    std::cout << "Binding parameter 3 (date): " << date << std::endl;

    /*
    // --- Real C++ DB Library (e.g., SOCI) Code would look like this ---
    //
    // session sql(postgresql, "dbname=weatherdb");
    // double temperature;
    // sql << "SELECT temperature FROM weather_data WHERE latitude = :lat AND longitude = :lon AND aqi_date = :date",
    //      use(latitude, "lat"), use(longitude, "lon"), use(date, "date"), into(temperature);
    //
    // This approach is safe from SQL injection.
    */
    
    // Using the mock database for a self-contained runnable example.
    std::cout << "Mock DB: Executing secure query..." << std::endl;
    return db.query(latitude, longitude, date);
}

void run_test(int test_num, MockDatabase& db, double lat, double lon, const std::string& date) {
    std::optional<double> temp = temperature_for_location(db, lat, lon, date);
    std::cout << "Result " << test_num << ": ";
    if (temp.has_value()) {
        std::cout << temp.value() << std::endl;
    } else {
        std::cout << "Not Found" << std::endl;
    }
}

int main() {
    MockDatabase db;
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Valid and existing data
    run_test(1, db, 40.71, -74.00, "2023-10-26");

    // Test Case 2: Valid and existing data
    run_test(2, db, 48.85, 2.35, "2023-10-27");

    // Test Case 3: Data that does not exist
    run_test(3, db, 0.0, 0.0, "2000-01-01");

    // Test Case 4: Malicious-looking input (which is safely handled by prepared statements)
    run_test(4, db, 0.0, 0.0, "2023-10-27' OR '1'='1");

    // Test Case 5: Another valid case
    run_test(5, db, 35.68, 139.69, "2023-10-27");

    return 0;
}