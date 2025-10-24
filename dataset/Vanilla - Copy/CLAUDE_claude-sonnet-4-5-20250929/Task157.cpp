
#include <iostream>
#include <cstdlib>
#include <cstring>

class MemoryResource {
private:
    int* data;
    int size;
    
public:
    MemoryResource(int size) : size(size) {
        data = new int[size];
        memset(data, 0, size * sizeof(int));
        std::cout << "Allocated memory for " << size << " integers" << std::endl;
    }
    
    void setData(int index, int value) {
        if (index >= 0 && index < size) {
            data[index] = value;
        }
    }
    
    int getData(int index) {
        if (index >= 0 && index < size) {
            return data[index];
        }
        return -1;
    }
    
    void deallocate() {
        if (data != nullptr) {
            delete[] data;
            data = nullptr;
            std::cout << "Deallocated memory for " << size << " integers" << std::endl;
        }
    }
    
    int getSize() const {
        return size;
    }
    
    ~MemoryResource() {
        if (data != nullptr) {
            delete[] data;
        }
    }
};

MemoryResource* allocateMemory(int size) {
    return new MemoryResource(size);
}

void deallocateMemory(MemoryResource* resource) {
    if (resource != nullptr) {
        resource->deallocate();
        delete resource;
    }
}

int main() {
    std::cout << "Test Case 1: Allocate and deallocate 10 integers" << std::endl;
    MemoryResource* res1 = allocateMemory(10);
    res1->setData(0, 100);
    res1->setData(5, 500);
    std::cout << "Data at index 0: " << res1->getData(0) << std::endl;
    std::cout << "Data at index 5: " << res1->getData(5) << std::endl;
    deallocateMemory(res1);
    std::cout << std::endl;
    
    std::cout << "Test Case 2: Allocate and deallocate 100 integers" << std::endl;
    MemoryResource* res2 = allocateMemory(100);
    for (int i = 0; i < 10; i++) {
        res2->setData(i, i * 10);
    }
    std::cout << "Data at index 7: " << res2->getData(7) << std::endl;
    deallocateMemory(res2);
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Allocate and deallocate 1 integer" << std::endl;
    MemoryResource* res3 = allocateMemory(1);
    res3->setData(0, 999);
    std::cout << "Data at index 0: " << res3->getData(0) << std::endl;
    deallocateMemory(res3);
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Allocate and deallocate 1000 integers" << std::endl;
    MemoryResource* res4 = allocateMemory(1000);
    res4->setData(999, 12345);
    std::cout << "Data at index 999: " << res4->getData(999) << std::endl;
    deallocateMemory(res4);
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Multiple allocations and deallocations" << std::endl;
    MemoryResource* res5a = allocateMemory(50);
    MemoryResource* res5b = allocateMemory(75);
    res5a->setData(25, 2500);
    res5b->setData(50, 5000);
    std::cout << "Resource A at index 25: " << res5a->getData(25) << std::endl;
    std::cout << "Resource B at index 50: " << res5b->getData(50) << std::endl;
    deallocateMemory(res5a);
    deallocateMemory(res5b);
    
    return 0;
}
