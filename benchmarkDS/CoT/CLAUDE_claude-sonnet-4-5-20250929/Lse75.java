
class Lse75 {
    static class Person {
        int status;
        
        public Person() {
            this.status = 0;
        }
    }
    
    public static Person allocatePerson() {
        try {
            Person person = new Person();
            person.status = 0;
            return person;
        } catch (OutOfMemoryError e) {
            System.err.println("Memory allocation failed: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal allocation
        Person p1 = allocatePerson();
        if (p1 != null) {
            System.out.println("Test 1 - Status: " + p1.status);
        }
        
        // Test case 2: Multiple allocations
        Person p2 = allocatePerson();
        if (p2 != null) {
            System.out.println("Test 2 - Status: " + p2.status);
        }
        
        // Test case 3: Verify independent objects
        Person p3 = allocatePerson();
        if (p3 != null) {
            p3.status = 5;
            System.out.println("Test 3 - Modified status: " + p3.status);
        }
        
        // Test case 4: Check original unchanged
        if (p1 != null) {
            System.out.println("Test 4 - Original status: " + p1.status);
        }
        
        // Test case 5: Multiple allocations in loop
        System.out.println("Test 5 - Allocating 5 persons:");
        for (int i = 0; i < 5; i++) {
            Person p = allocatePerson();
            if (p != null) {
                System.out.println("  Person " + (i+1) + " status: " + p.status);
            }
        }
    }
}
