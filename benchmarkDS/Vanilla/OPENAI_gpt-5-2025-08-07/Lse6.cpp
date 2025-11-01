#include <iostream>
#include <vector>
#include <string>
using namespace std;

class Lse6 {
public:
    static string getVehicleByIndex(const vector<string>& vehicles, int index) {
        if (index < 0 || index >= static_cast<int>(vehicles.size())) return "Invalid index";
        return vehicles[index];
    }

    static void printVehicles(const vector<string>& vehicles) {
        for (const auto& v : vehicles) {
            cout << v << "\n";
        }
    }
};

int main() {
    vector<string> vehicles = {"Car", "Bus", "Bike", "Truck", "Scooter"};

    vector<int> testIndices = {0, 2, 4, -1, 10};
    for (int idx : testIndices) {
        string result = Lse6::getVehicleByIndex(vehicles, idx);
        cout << "Index " << idx << ": " << result << "\n";
    }

    cout << "All vehicles:\n";
    Lse6::printVehicles(vehicles);

    return 0;
}