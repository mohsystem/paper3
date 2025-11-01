
// Java: Memory is managed automatically. We create a Person object and set status to 0.
// The JVM handles allocation and garbage collection, preventing manual memory errors.

public class Lse78 {
    // Person class with status field
    static class Person {
        private int status;
        
        // Constructor initializes status to 0
        public Person() {
            this.status = 0;
        }
        
        public int getStatus() {
            return status;
        }
        
        public void setStatus(int status) {
            this.status = status;
        }
        
        @Override
        public String toString() {
            return "Person{status=" + status + "}";
        }
    }
    
    // Allocates a new Person with status set to 0
    public static Person allocatePerson() {
        // Java automatically allocates memory on the heap for new objects
        // No manual memory management required - GC handles cleanup
        Person newPerson = new Person();
        return newPerson;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic allocation
        Person p1 = allocatePerson();
        System.out.println("Test 1: " + p1);
        
        // Test case 2: Multiple allocations
        Person p2 = allocatePerson();
        Person p3 = allocatePerson();
        System.out.println("Test 2: " + p2 + ", " + p3);
        
        // Test case 3: Verify initial status
        Person p4 = allocatePerson();
        System.out.println("Test 3: Status is " + p4.getStatus());
        
        // Test case 4: Modify status after allocation
        Person p5 = allocatePerson();
        p5.setStatus(1);
        System.out.println("Test 4: Modified status to " + p5.getStatus());
        
        // Test case 5: Array of persons
        Person[] people = new Person[3];
        for (int i = 0; i < people.length; i++) {
            people[i] = allocatePerson();
        }
        System.out.println("Test 5: Allocated " + people.length + " persons");
        for (int i = 0; i < people.length; i++) {
            System.out.println("  Person " + i + ": " + people[i]);
        }
    }
}
