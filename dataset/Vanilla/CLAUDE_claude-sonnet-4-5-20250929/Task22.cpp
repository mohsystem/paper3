
#include <iostream>
#include <vector>
#include <string>
#include <variant>

using namespace std;

vector<int> filterList(const vector<variant<int, string>>& list) {
    vector<int> result;
    for (const auto& item : list) {
        if (holds_alternative<int>(item)) {
            result.push_back(get<int>(item));
        }
    }
    return result;
}

int main() {
    // Test case 1
    vector<variant<int, string>> test1 = {1, 2, string("a"), string("b")};
    vector<int> result1 = filterList(test1);
    cout << "[";
    for (size_t i = 0; i < result1.size(); i++) {
        cout << result1[i];
        if (i < result1.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
    
    // Test case 2
    vector<variant<int, string>> test2 = {1, string("a"), string("b"), 0, 15};
    vector<int> result2 = filterList(test2);
    cout << "[";
    for (size_t i = 0; i < result2.size(); i++) {
        cout << result2[i];
        if (i < result2.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
    
    // Test case 3
    vector<variant<int, string>> test3 = {1, 2, string("a"), string("b"), string("aasf"), string("1"), string("123"), 123};
    vector<int> result3 = filterList(test3);
    cout << "[";
    for (size_t i = 0; i < result3.size(); i++) {
        cout << result3[i];
        if (i < result3.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
    
    // Test case 4
    vector<variant<int, string>> test4 = {10, 20, 30, string("test"), string("hello"), 40};
    vector<int> result4 = filterList(test4);
    cout << "[";
    for (size_t i = 0; i < result4.size(); i++) {
        cout << result4[i];
        if (i < result4.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
    
    // Test case 5
    vector<variant<int, string>> test5 = {string("only"), string("strings"), string("here")};
    vector<int> result5 = filterList(test5);
    cout << "[";
    for (size_t i = 0; i < result5.size(); i++) {
        cout << result5[i];
        if (i < result5.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
    
    return 0;
}
