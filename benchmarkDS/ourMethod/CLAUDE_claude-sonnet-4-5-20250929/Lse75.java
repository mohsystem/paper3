
public class Lse75 {
    static class Person {
        int status;
        
        public Person() {
            this.status = 0;
        }
    }
    
    // Allocates a new Person object with status initialized to 0
    // Returns null on allocation failure (simulated by catching OutOfMemoryError)
    public static Person allocatePerson() {
        try {
            Person p = new Person();
            // Explicit initialization to 0 (already done in constructor)
            p.status = 0;
            return p;
        } catch (OutOfMemoryError e) {
            // Return null on allocation failure
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal allocation
        Person p1 = allocatePerson();
        if (p1 != null) {
            System.out.println("Test 1 - Person allocated, status: " + p1.status);
        } else {
            System.out.println("Test 1 - Allocation failed");
        }
        
        // Test case 2: Verify status is 0
        Person p2 = allocatePerson();
        if (p2 != null) {
            System.out.println("Test 2 - Status is zero: " + (p2.status == 0));
        }
        
        // Test case 3: Multiple allocations
        Person p3 = allocatePerson();
        if (p3 != null) {
            System.out.println("Test 3 - Multiple allocations work, status: " + p3.status);
        }
        
        // Test case 4: Verify independence of objects
        Person p4 = allocatePerson();
        Person p5 = allocatePerson();
        if (p4 != null && p5 != null) {
            p4.status = 1;
            System.out.println("Test 4 - Objects are independent: " + (p5.status == 0));
        }
        
        // Test case 5: Null check handling
        Person p6 = allocatePerson();
        System.out.println("Test 5 - Null check passed: " + (p6 != null));
    }
}
