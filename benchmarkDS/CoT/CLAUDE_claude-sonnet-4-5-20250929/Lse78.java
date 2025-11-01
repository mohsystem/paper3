
public class Lse78 {
    static class Person {
        private int status;
        
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
    
    public static Person allocatePerson() {
        try {
            Person person = new Person();
            if (person == null) {
                throw new OutOfMemoryError("Failed to allocate Person");
            }
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
            System.out.println("Test 1 - Status: " + p1.getStatus());
        }
        
        // Test case 2: Multiple allocations
        Person p2 = allocatePerson();
        if (p2 != null) {
            System.out.println("Test 2 - Status: " + p2.getStatus());
        }
        
        // Test case 3: Verify independence
        if (p1 != null) {
            p1.setStatus(5);
        }
        if (p2 != null) {
            System.out.println("Test 3 - P2 Status (should be 0): " + p2.getStatus());
        }
        
        // Test case 4: Another allocation
        Person p3 = allocatePerson();
        if (p3 != null) {
            System.out.println("Test 4 - Status: " + p3.getStatus());
        }
        
        // Test case 5: Verify initialization
        Person p4 = allocatePerson();
        if (p4 != null) {
            System.out.println("Test 5 - Status: " + p4.getStatus());
        }
    }
}
