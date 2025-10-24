
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

class Task19 {
public:
    static string spinWords(string sentence) {
        stringstream ss(sentence);
        string word;
        vector<string> words;
        
        while (ss >> word) {
            words.push_back(word);
        }
        
        string result = "";
        for (int i = 0; i < words.size(); i++) {
            if (words[i].length() >= 5) {
                string reversed = words[i];
                reverse(reversed.begin(), reversed.end());
                result += reversed;
            } else {
                result += words[i];
            }
            
            if (i < words.size() - 1) {
                result += " ";
            }
        }
        
        return result;
    }
};

int main() {
    // Test case 1
    cout << Task19::spinWords("Hey fellow warriors") << endl;
    
    // Test case 2
    cout << Task19::spinWords("This is a test") << endl;
    
    // Test case 3
    cout << Task19::spinWords("This is another test") << endl;
    
    // Test case 4
    cout << Task19::spinWords("Welcome") << endl;
    
    // Test case 5
    cout << Task19::spinWords("Hi") << endl;
    
    return 0;
}
