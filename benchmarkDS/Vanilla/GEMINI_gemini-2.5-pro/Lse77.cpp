#include <iostream>
#include <string>

// The Lse77 class is not strictly necessary for this example in C++
// but is included to meet the naming requirement if it were a full application.
class Lse77 {
public:
    struct Person {
        int status;
        std::string name;
    };

    /**
     * Allocates a block of memory for a Person struct on the heap
     * and returns a pointer to the beginning of the block.
     * @return A pointer to a new Person struct with name "John" and status 0.
     */
    static Person* createPerson() {
        // 'new' allocates memory on the heap.
        Person* p = new Person();
        // Sets the status to 0 and the name to "John".
        p->status = 0;
        p->name = "John";
        return p;
    }
};

int main() {
    std::cout << "Running CPP Test Cases:" << std::endl;

    // Test Case 1
    Lse77::Person* p1 = Lse77::createPerson();
    std::cout << "Test Case 1: Person{name='" << p1->name << "', status=" << p1->status << "}" << std::endl;
    delete p1; // Free the allocated memory

    // Test Case 2
    Lse77::Person* p2 = Lse77::createPerson();
    std::cout << "Test Case 2: Person{name='" << p2->name << "', status=" << p2->status << "}" << std::endl;
    delete p2;

    // Test Case 3
    Lse77::Person* p3 = Lse77::createPerson();
    std::cout << "Test Case 3: Person{name='" << p3->name << "', status=" << p3->status << "}" << std::endl;
    delete p3;

    // Test Case 4
    Lse77::Person* p4 = Lse77::createPerson();
    std::cout << "Test Case 4: Person{name='" << p4->name << "', status=" << p4->status << "}" << std::endl;
    delete p4;

    // Test Case 5
    Lse77::Person* p5 = Lse77::createPerson();
    std::cout << "Test Case 5: Person{name='" << p5->name << "', status=" << p5->status << "}" << std::endl;
    delete p5;

    return 0;
}