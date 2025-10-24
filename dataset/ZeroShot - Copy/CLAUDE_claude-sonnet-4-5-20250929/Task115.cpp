
#include <iostream>
#include <cstdint>

struct Task115 {
    uint8_t flag1 : 1;
    uint8_t flag2 : 1;
    uint8_t value : 4;
    uint8_t status : 2;
    
    Task115() : flag1(0), flag2(0), value(0), status(0) {}
    
    void setFlag1(bool val) {
        flag1 = val ? 1 : 0;
    }
    
    bool getFlag1() const {
        return flag1 != 0;
    }
    
    void setFlag2(bool val) {
        flag2 = val ? 1 : 0;
    }
    
    bool getFlag2() const {
        return flag2 != 0;
    }
    
    void setValue(int val) {
        if (val < 0 || val > 15) {
            std::cerr << "Error: Value must be between 0 and 15" << std::endl;
            return;
        }
        value = val;
    }
    
    int getValue() const {
        return value;
    }
    
    void setStatus(int stat) {
        if (stat < 0 || stat > 3) {
            std::cerr << "Error: Status must be between 0 and 3" << std::endl;
            return;
        }
        status = stat;
    }
    
    int getStatus() const {
        return status;
    }
    
    void print() const {
        std::cout << "BitField{flag1=" << getFlag1() 
                  << ", flag2=" << getFlag2() 
                  << ", value=" << getValue() 
                  << ", status=" << getStatus() << "}" << std::endl;
    }
};

int main() {
    std::cout << "Test Case 1: Initialize and check default values" << std::endl;
    Task115 bf1;
    bf1.print();
    
    std::cout << "\\nTest Case 2: Set all flags and values" << std::endl;
    Task115 bf2;
    bf2.setFlag1(true);
    bf2.setFlag2(true);
    bf2.setValue(10);
    bf2.setStatus(2);
    bf2.print();
    
    std::cout << "\\nTest Case 3: Set partial values" << std::endl;
    Task115 bf3;
    bf3.setFlag1(true);
    bf3.setValue(15);
    bf3.print();
    
    std::cout << "\\nTest Case 4: Toggle flags" << std::endl;
    Task115 bf4;
    bf4.setFlag1(true);
    bf4.setFlag2(false);
    bf4.setValue(7);
    bf4.setStatus(3);
    bf4.print();
    bf4.setFlag1(false);
    bf4.setFlag2(true);
    bf4.print();
    
    std::cout << "\\nTest Case 5: Maximum values" << std::endl;
    Task115 bf5;
    bf5.setFlag1(true);
    bf5.setFlag2(true);
    bf5.setValue(15);
    bf5.setStatus(3);
    bf5.print();
    
    return 0;
}
