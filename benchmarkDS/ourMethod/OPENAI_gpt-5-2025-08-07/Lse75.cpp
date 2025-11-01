#include <iostream>
#include <memory>
#include <string>
#include <cctype>

struct Person {
    std::string name;
    int age;
    int status; // will be set to 0 on allocation
};

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

// Allocates a new Person and sets status to 0
std::unique_ptr<Person> allocate_person(const std::string& name, int age) {
    if (age < 0 || age > 150) {
        return nullptr; // Fail closed on invalid age
    }
    std::unique_ptr<Person> p = std::make_unique<Person>();
    std::string t = trim(name);
    if (t.empty()) t = "unknown";
    p->name = t;
    p->age = age;
    p->status = 0; // explicitly set status to 0
    return p;
}

int main() {
    // 5 test cases
    auto p1 = allocate_person("Alice", 30);
    auto p2 = allocate_person("Bob", 45);
    auto p3 = allocate_person("Charlie", 0);
    auto p4 = allocate_person("Dana", 99);
    auto p5 = allocate_person("", 25);

    std::cout << (p1 ? "Person{name='" + p1->name + "', age=" + std::to_string(p1->age) + ", status=" + std::to_string(p1->status) + "}" : "Allocation failed") << "\n";
    std::cout << (p2 ? "Person{name='" + p2->name + "', age=" + std::to_string(p2->age) + ", status=" + std::to_string(p2->status) + "}" : "Allocation failed") << "\n";
    std::cout << (p3 ? "Person{name='" + p3->name + "', age=" + std::to_string(p3->age) + ", status=" + std::to_string(p3->status) + "}" : "Allocation failed") << "\n";
    std::cout << (p4 ? "Person{name='" + p4->name + "', age=" + std::to_string(p4->age) + ", status=" + std::to_string(p4->status) + "}" : "Allocation failed") << "\n";
    std::cout << (p5 ? "Person{name='" + p5->name + "', age=" + std::to_string(p5->age) + ", status=" + std::to_string(p5->status) + "}" : "Allocation failed") << "\n";

    return 0;
}