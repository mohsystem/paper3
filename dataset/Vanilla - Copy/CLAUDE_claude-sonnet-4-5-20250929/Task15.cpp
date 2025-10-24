
#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> buildTower(int nFloors) {
    std::vector<std::string> tower;
    int maxWidth = 2 * nFloors - 1;
    
    for (int i = 0; i < nFloors; i++) {
        int stars = 2 * i + 1;
        int spaces = (maxWidth - stars) / 2;
        
        std::string floor = "";
        for (int j = 0; j < spaces; j++) {
            floor += " ";
        }
        for (int j = 0; j < stars; j++) {
            floor += "*";
        }
        for (int j = 0; j < spaces; j++) {
            floor += " ";
        }
        
        tower.push_back(floor);
    }
    
    return tower;
}

int main() {
    // Test case 1
    std::cout << "Test case 1: 3 floors" << std::endl;
    std::vector<std::string> tower1 = buildTower(3);
    for (const std::string& floor : tower1) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test case 2: 6 floors" << std::endl;
    std::vector<std::string> tower2 = buildTower(6);
    for (const std::string& floor : tower2) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test case 3: 1 floor" << std::endl;
    std::vector<std::string> tower3 = buildTower(1);
    for (const std::string& floor : tower3) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test case 4: 5 floors" << std::endl;
    std::vector<std::string> tower4 = buildTower(5);
    for (const std::string& floor : tower4) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test case 5: 10 floors" << std::endl;
    std::vector<std::string> tower5 = buildTower(10);
    for (const std::string& floor : tower5) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    
    return 0;
}
