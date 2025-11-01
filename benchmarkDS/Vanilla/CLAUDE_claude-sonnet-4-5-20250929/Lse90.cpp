
#include <iostream>
#include <string>
#include <map>
#include <sstream>

using namespace std;

double getPriceForProductCode(const string& productCode) {
    map<string, double> prices;
    prices["PROD001"] = 10.99;
    prices["PROD002"] = 25.50;
    prices["PROD003"] = 5.75;
    prices["PROD004"] = 100.00;
    prices["PROD005"] = 15.25;
    
    if (prices.find(productCode) != prices.end()) {
        return prices[productCode];
    }
    return 0.0;
}

string calculatePrice(const string& yamlPayload) {
    try {
        string productCode;
        int quantity = 0;
        
        istringstream iss(yamlPayload);
        string line;
        
        while (getline(iss, line)) {
            size_t pos = line.find("product_code:");
            if (pos != string::npos) {
                productCode = line.substr(pos + 13);
                productCode.erase(0, productCode.find_first_not_of(" \\t"));
            }
            
            pos = line.find("quantity:");
            if (pos != string::npos) {
                string qtyStr = line.substr(pos + 9);
                qtyStr.erase(0, qtyStr.find_first_not_of(" \\t"));
                quantity = stoi(qtyStr);
            }
        }
        
        double price = getPriceForProductCode(productCode);
        double totalPrice = price * quantity;
        
        ostringstream result;
        result << "product_code: " << productCode << "\\n";
        result << "quantity: " << quantity << "\\n";
        result << "total_price: " << totalPrice << "\\n";
        
        return result.str();
    } catch (const exception& e) {
        return string("error: ") + e.what();
    }
}

int main() {
    // Test case 1
    string yaml1 = "product_code: PROD001\\nquantity: 2";
    cout << "Test 1:\\n" << calculatePrice(yaml1) << endl;
    
    // Test case 2
    string yaml2 = "product_code: PROD002\\nquantity: 5";
    cout << "Test 2:\\n" << calculatePrice(yaml2) << endl;
    
    // Test case 3
    string yaml3 = "product_code: PROD003\\nquantity: 10";
    cout << "Test 3:\\n" << calculatePrice(yaml3) << endl;
    
    // Test case 4
    string yaml4 = "product_code: PROD004\\nquantity: 1";
    cout << "Test 4:\\n" << calculatePrice(yaml4) << endl;
    
    // Test case 5
    string yaml5 = "product_code: PROD999\\nquantity: 3";
    cout << "Test 5:\\n" << calculatePrice(yaml5) << endl;
    
    return 0;
}
