
// INSECURE - For educational purposes only
// WARNING: Never store sensitive data like this in production!

#include <stdio.h>
#include <string.h>

typedef struct {
    char creditCard[20];
    char ssn[12];
    char name[50];
} Task124;

void displayData(Task124 data) {
    printf("Name: %s, CC: %s, SSN: %s\\n", data.name, data.creditCard, data.ssn);
}

int main() {
    printf("WARNING: This is an INSECURE example for educational purposes only!\\n");
    printf("Never store sensitive data in plain text in production systems.\\n\\n");
    
    // Test cases showing insecure storage
    Task124 data1 = {"4532-1234-5678-9010", "123-45-6789", "John Doe"};
    Task124 data2 = {"5425-2334-3010-9876", "987-65-4321", "Jane Smith"};
    Task124 data3 = {"3782-822463-10005", "555-55-5555", "Bob Johnson"};
    Task124 data4 = {"6011-1111-1111-1117", "222-22-2222", "Alice Williams"};
    Task124 data5 = {"3056-9309-0259-04", "333-33-3333", "Charlie Brown"};
    
    printf("Insecurely stored data:\\n");
    displayData(data1);
    displayData(data2);
    displayData(data3);
    displayData(data4);
    displayData(data5);
    
    return 0;
}
