
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <limits>

class MemoryResource {
private:
    size_t size;
    unsigned char* data;
    bool allocated;
    
public:
    MemoryResource(size_t size) : size(size), data(nullptr), allocated(false) {
        if (size == 0) {
            throw std::invalid_argument("Size must be positive");
        }
        if (size > std::numeric_limits<size_t>::max() / 2) {
            throw std::invalid_argument("Size too large");
        }
        data = new unsigned char[size];
        std::memset(data, 0, size);
        allocated = true;
    }
    
    ~MemoryResource() {
        if (allocated && data != nullptr) {
            deallocate();
        }
    }
    
    // Prevent copying to avoid double-free
    MemoryResource(const MemoryResource&) = delete;
    MemoryResource& operator=(const MemoryResource&) = delete;
    
    void deallocate() {
        if (!allocated) {
            throw std::runtime_error("Resource already deallocated");
        }
        if (data != nullptr) {
            // Clear sensitive data before deallocation
            std::memset(data, 0, size);
            delete[] data;
            data = nullptr;
        }
        allocated = false;
    }
    
    bool isAllocated() const {
        return allocated;
    }
    
    size_t getSize() const {
        return size;
    }
};

MemoryResource* allocateMemory(size_t size) {
    if (size == 0 || size > std::numeric_limits<size_t>::max() / 2) {
        throw std::invalid_argument("Invalid memory size");
    }
    return new MemoryResource(size);
}

void deallocateMemory(MemoryResource* resource) {
    if (resource == nullptr) {
        throw std::invalid_argument("Resource cannot be null");
    }
    resource->deallocate();
    delete resource;
}

int main() {
    std::cout << "Test Case 1: Basic allocation and deallocation" << std::endl;
    try {
        MemoryResource* res1 = allocateMemory(1024);
        std::cout << "Allocated: " << res1->getSize() << " bytes, Status: " << res1->isAllocated() << std::endl;
        deallocateMemory(res1);
        std::cout << "Deallocated successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 2: Multiple allocations" << std::endl;
    try {
        std::vector<MemoryResource*> resources;
        for (int i = 1; i <= 3; i++) {
            MemoryResource* res = allocateMemory(512 * i);
            resources.push_back(res);
            std::cout << "Allocated resource " << i << ": " << res->getSize() << " bytes" << std::endl;
        }
        for (auto res : resources) {
            deallocateMemory(res);
        }
        std::cout << "All resources deallocated" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 3: Invalid size (zero)" << std::endl;
    try {
        MemoryResource* res3 = allocateMemory(0);
    } catch (const std::exception& e) {
        std::cout << "Caught expected error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 4: Double deallocation prevention" << std::endl;
    try {
        MemoryResource* res4 = allocateMemory(2048);
        res4->deallocate();
        res4->deallocate();
    } catch (const std::exception& e) {
        std::cout << "Caught expected error: " << e.what() << std::endl;
        delete res4;
    }
    
    std::cout << "\\nTest Case 5: Null resource deallocation" << std::endl;
    try {
        deallocateMemory(nullptr);
    } catch (const std::exception& e) {
        std::cout << "Caught expected error: " << e.what() << std::endl;
    }
    
    return 0;
}
