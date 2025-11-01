
#include <iostream>
#include <cstdlib>

struct Person {
    int status;
    char name[50];
    int age;
};

Person* allocatePerson() {
    Person* person = (Person*)malloc(sizeof(Person));
    if (person != NULL) {
        person->status = 0;
    }
    return person;
}

int main() {
    // Test case 1
    Person* p1 = allocatePerson();
    std::cout << "Test 1 - Status: " << p1->status << std::endl;
    
    // Test case 2
    Person* p2 = allocatePerson();
    std::cout << "Test 2 - Status: " << p2->status << std::endl;
    
    // Test case 3
    Person* p3 = allocatePerson();
    std::cout << "Test 3 - Status: " << p3->status << std::endl;
    
    // Test case 4
    Person* p4 = allocatePerson();
    std::cout << "Test 4 - Status: " << p4->status << std::endl;
    
    // Test case 5
    Person* p5 = allocatePerson();
    std::cout << "Test 5 - Status: " << p5->status << std::endl;
    
    // Free allocated memory
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    
    return 0;
}
