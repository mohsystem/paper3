
class Lse76 {
    static class Person {
        int status;
        String name;
        int age;
        
        public Person() {
            this.status = 0;
        }
    }
    
    public static Person allocatePerson() {
        Person person = new Person();
        person.status = 0;
        return person;
    }
    
    public static void main(String[] args) {
        // Test case 1: Allocate single person
        Person p1 = allocatePerson();
        System.out.println("Test 1 - Person status: " + p1.status);
        
        // Test case 2: Allocate multiple persons
        Person p2 = allocatePerson();
        Person p3 = allocatePerson();
        System.out.println("Test 2 - Person2 status: " + p2.status + ", Person3 status: " + p3.status);
        
        // Test case 3: Verify independence
        p1.status = 5;
        Person p4 = allocatePerson();
        System.out.println("Test 3 - Person1 status: " + p1.status + ", Person4 status: " + p4.status);
        
        // Test case 4: Allocate and set properties
        Person p5 = allocatePerson();
        p5.name = "John";
        p5.age = 30;
        System.out.println("Test 4 - Person5 status: " + p5.status + ", Name: " + p5.name);
        
        // Test case 5: Multiple allocations in loop
        for (int i = 0; i < 3; i++) {
            Person p = allocatePerson();
            System.out.println("Test 5 - Iteration " + i + " status: " + p.status);
        }
    }
}
