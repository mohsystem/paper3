#include <iostream>
#include <memory>
#include <string>
#include <algorithm>

static constexpr std::size_t MAX_NAME_LEN = 64;

struct Person {
    std::string name;
    int age;
    int status; // must be 0 for new allocations
};

static std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static int sanitize_age(int age) {
    if (age < 0) return 0;
    if (age > 150) return 150;
    return age;
}

std::unique_ptr<Person> allocate_person(const std::string& name, int age) {
    std::string n = trim(name);
    if (n.size() > MAX_NAME_LEN) {
        n = n.substr(0, MAX_NAME_LEN);
    }

    auto p = std::make_unique<Person>();
    p->name = n;
    p->age = sanitize_age(age);
    p->status = 0; // explicitly set status to 0
    return p; // returns pointer-like handle to the allocated block
}

int main() {
    // 5 test cases
    auto p1 = allocate_person("Alice", 30);
    auto p2 = allocate_person(" Bob  ", -5);
    auto p3 = allocate_person("", 151);
    auto p4 = allocate_person("ThisNameIsWayTooLongAndShouldBeTruncatedAtSixtyFourCharacters_ABCDEFGHIJKLMNOPQRSTUVWXYZ", 42);
    auto p5 = allocate_person("   ", 0);

    auto print = [](const std::unique_ptr<Person>& p) {
        if (!p) {
            std::cout << "Allocation failed\n";
            return;
        }
        std::cout << "Person{name='" << p->name << "', age=" << p->age << ", status=" << p->status << "}\n";
    };

    print(p1);
    print(p2);
    print(p3);
    print(p4);
    print(p5);

    return 0;
}