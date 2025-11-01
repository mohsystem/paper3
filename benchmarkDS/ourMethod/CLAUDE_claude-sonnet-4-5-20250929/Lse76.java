
public class Lse76 {
    // Person class to represent a person with status
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
    }
    
    // Memory allocation method that creates a new Person and sets status to 0
    // Returns the newly allocated Person object
    public static Person allocatePerson() {
        // Allocate new Person object (Java handles memory allocation automatically)
        Person newPerson = new Person();
        // Status is already initialized to 0 in constructor
        return newPerson;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic allocation
        Person p1 = allocatePerson();
        System.out.println("Test 1 - Status: " + p1.getStatus());
        
        // Test case 2: Multiple allocations
        Person p2 = allocatePerson();
        System.out.println("Test 2 - Status: " + p2.getStatus());
        
        // Test case 3: Verify independence
        p1.setStatus(5);
        Person p3 = allocatePerson();
        System.out.println("Test 3 - p1 Status: " + p1.getStatus() + ", p3 Status: " + p3.getStatus());
        
        // Test case 4: Array of persons
        Person[] persons = new Person[3];
        for (int i = 0; i < persons.length; i++) {
            persons[i] = allocatePerson();
            System.out.println("Test 4." + (i+1) + " - Status: " + persons[i].getStatus());
        }
        
        // Test case 5: Null safety check
        Person p5 = allocatePerson();
        if (p5 != null) {
            System.out.println("Test 5 - Allocation successful, Status: " + p5.getStatus());
        }
    }
}
