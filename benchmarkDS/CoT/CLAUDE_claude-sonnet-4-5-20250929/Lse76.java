
public class Lse76 {
    static class Person {
        int status;
        String name;
        int age;
        
        public Person() {
            this.status = 0;
            this.name = "";
            this.age = 0;
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
            System.out.println("Test 1 - Person allocated, status: " + p1.status);
        }
        
        // Test case 2: Verify status is 0
        Person p2 = allocatePerson();
        if (p2 != null) {
            System.out.println("Test 2 - Status is zero: " + (p2.status == 0));
        }
        
        // Test case 3: Multiple allocations
        Person p3 = allocatePerson();
        if (p3 != null) {
            System.out.println("Test 3 - Multiple allocation successful, status: " + p3.status);
        }
        
        // Test case 4: Modify and verify independence
        Person p4 = allocatePerson();
        if (p4 != null) {
            p4.status = 5;
            System.out.println("Test 4 - Modified status: " + p4.status);
        }
        
        // Test case 5: Verify new allocation is still 0
        Person p5 = allocatePerson();
        if (p5 != null) {
            System.out.println("Test 5 - New allocation status still 0: " + (p5.status == 0));
        }
    }
}
