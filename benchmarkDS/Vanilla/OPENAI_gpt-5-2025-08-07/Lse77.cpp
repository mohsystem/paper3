#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

struct Person {
    int status;
    std::string name;
    Person(int s, const std::string& n) : status(s), name(n) {}
};

void* alloc(std::size_t size) {
    return std::malloc(size);
}

Person createPerson(const std::string& name, int status) {
    return Person(status, name);
}

int main() {
    std::size_t sizes[5] = {8, 16, 32, 64, 128};
    void* blocks[5] = {nullptr};

    for (int i = 0; i < 5; ++i) {
        blocks[i] = alloc(sizes[i]);
        std::cout << "Allocated block " << i + 1 << " of size " << sizes[i]
                  << " at address " << blocks[i] << std::endl;
    }

    Person john = createPerson("John", 0);
    std::cout << "Created person: name=" << john.name << ", status=" << john.status << std::endl;

    // Additional small validations
    if (blocks[0] && sizes[0] >= 3) {
        unsigned char* p = static_cast<unsigned char*>(blocks[0]);
        p[0] = 1; p[1] = 2; p[2] = 3;
        std::cout << "Sample bytes: " << (int)p[0] << "," << (int)p[1] << "," << (int)p[2] << std::endl;
    }

    for (void* ptr : blocks) {
        std::free(ptr);
    }
    return 0;
}