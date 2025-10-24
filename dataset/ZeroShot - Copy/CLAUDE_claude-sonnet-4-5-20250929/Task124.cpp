
// INSECURE - For educational purposes only
// WARNING: Never store sensitive data like this in production!

#include <iostream>
#include <string>
using namespace std;

class Task124 {
private:
    string creditCard;
    string ssn;
    string name;
    
public:
    Task124(string cc, string s, string n) : creditCard(cc), ssn(s), name(n) {}
    
    void display() {
        cout << "Name: " << name << ", CC: " << creditCard << ", SSN: " << ssn << endl;
    }
};

int main() {
    cout << "WARNING: This is an INSECURE example for educational purposes only!" << endl;
    cout << "Never store sensitive data in plain text in production systems.\\n" << endl;
    
    // Test cases showing insecure storage
    Task124 data1("4532-1234-5678-9010", "123-45-6789", "John Doe");
    Task124 data2("5425-2334-3010-9876", "987-65-4321", "Jane Smith");
    Task124 data3("3782-822463-10005", "555-55-5555", "Bob Johnson");
    Task124 data4("6011-1111-1111-1117", "222-22-2222", "Alice Williams");
    Task124 data5("3056-9309-0259-04", "333-33-3333", "Charlie Brown");
    
    cout << "Insecurely stored data:" << endl;
    data1.display();
    data2.display();
    data3.display();
    data4.display();
    data5.display();
    
    return 0;
}
