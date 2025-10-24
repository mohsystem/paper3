
#include <iostream>
#include <cstdlib>
#include <cstring>

class MemoryResource {
private:
    void* data;
    size_t size;
    bool allocated;
    
public:
    MemoryResource(size_t s) : size(s), allocated(false) {
        if (s == 0) {
            throw std::invalid_argument("Size must be positive");
        }
        data = malloc(s);
        if (data == nullptr) {
            throw std::bad_alloc();
        }
        memset(data, 0, s);
        allocated = true;
        std::cout << "Memory allocated: " << s << " bytes" << std::endl;
    }
    
    ~MemoryResource() {
        if (allocated) {
            deallocate();
        }
    }
    
    void deallocate() {
        if (allocated && data != nullptr) {
            free(data);
            data = nullptr;
            allocated = false;
            std::cout << "Memory deallocated" << std::endl;
        }
    }
    
    bool isAllocated() const {
        return allocated;
    }
    
    size_t getSize() const {
        return allocated ? size : 0;
    }
};

MemoryResource* allocateMemory(size_t size) {
    return new MemoryResource(size);
}

void deallocateMemory(MemoryResource* resource) {
    if (resource != nullptr) {
        resource->deallocate();
        delete resource;
    }
}

int main() {
    std::cout << "Test Case 1: Allocate and deallocate 1024 bytes" << std::endl;
    MemoryResource* res1 = allocateMemory(1024);
    std::cout << "Is allocated: " << res1->isAllocated() << std::endl;
    deallocateMemory(res1);
    std::cout << std::endl;
    
    std::cout << "Test Case 2: Allocate and deallocate 2048 bytes" << std::endl;
    MemoryResource* res2 = allocateMemory(2048);
    std::cout << "Size: " << res2->getSize() << std::endl;
    deallocateMemory(res2);
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Multiple allocations" << std::endl;
    MemoryResource* res3 = allocateMemory(512);
    MemoryResource* res4 = allocateMemory(256);
    deallocateMemory(res3);
    deallocateMemory(res4);
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Deallocate null resource" << std::endl;
    deallocateMemory(nullptr);
    std::cout << "No error on null deallocation" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Large allocation" << std::endl;
    MemoryResource* res5 = allocateMemory(1048576); // 1MB
    std::cout << "Large allocation size: " << res5->getSize() << std::endl;
    deallocateMemory(res5);
    
    return 0;
}
