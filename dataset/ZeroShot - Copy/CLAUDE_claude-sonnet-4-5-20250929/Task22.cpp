
#include <iostream>
#include <vector>
#include <string>
#include <variant>

using namespace std;

class Task22 {
public:
    static vector<int> filterList(const vector<variant<int, string>>& list) {
        vector<int> result;
        
        for (const auto& item : list) {
            if (holds_alternative<int>(item)) {
                result.push_back(get<int>(item));
            }
        }
        
        return result;
    }
    
    static void printVector(const vector<int>& vec) {
        cout << "[";
        for (size_t i = 0; i < vec.size(); i++) {
            cout << vec[i];
            if (i < vec.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }
};

int main() {
    // Test case 1
    vector<variant<int, string>> test1 = {1, 2, string("a"), string("b")};
    cout << "Test 1: ";
    Task22::printVector(Task22::filterList(test1));
    
    // Test case 2
    vector<variant<int, string>> test2 = {1, string("a"), string("b"), 0, 15};
    cout << "Test 2: ";
    Task22::printVector(Task22::filterList(test2));
    
    // Test case 3
    vector<variant<int, string>> test3 = {1, 2, string("a"), string("b"), string("aasf"), string("1"), string("123"), 123};
    cout << "Test 3: ";
    Task22::printVector(Task22::filterList(test3));
    
    // Test case 4
    vector<variant<int, string>> test4 = {5, string("test"), 10, string("string"), 20};
    cout << "Test 4: ";
    Task22::printVector(Task22::filterList(test4));
    
    // Test case 5
    vector<variant<int, string>> test5 = {100, 200, 300};
    cout << "Test 5: ";
    Task22::printVector(Task22::filterList(test5));
    
    return 0;
}
