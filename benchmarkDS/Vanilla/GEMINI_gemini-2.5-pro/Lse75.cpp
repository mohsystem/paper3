#include <iostream>
#include <vector>

struct Person {
    int id;
    int status;

    // Constructor to initialize status to 0 upon creation
    Person(int person_id) : id(person_id), status(0) {}
};

/**
 * Allocates a block of memory for a Person using 'new'
 * and returns a pointer to the beginning of the block.
 * The constructor sets the status to 0.
 * @return A pointer to the newly allocated Person.
 */
Person* allocatePerson(int id) {
    return new Person(id);
}

int main() {
    std::cout << "CPP Test Cases:" << std::endl;
    std::vector<Person*> people;

    // 5 Test cases
    for (int i = 0; i < 5; ++i) {
        Person* p = allocatePerson(i + 1);
        people.push_back(p);
        std::cout << "Test Case " << i + 1 << ": Allocated Person at " << p
                  << " with id=" << p->id << ", status=" << p->status << std::endl;
    }

    // Clean up the dynamically allocated memory to prevent memory leaks
    std::cout << "\nCleaning up memory..." << std::endl;
    for (Person* p : people) {
        delete p;
    }
    people.clear();

    return 0;
}