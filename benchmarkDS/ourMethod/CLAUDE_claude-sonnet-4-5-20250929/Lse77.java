
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse77 {
    // Person class to represent the data structure
    static class Person {
        private int status;
        private String name;
        
        // Constructor with validation
        public Person(int status, String name) {
            // Input validation: check name is not null and within reasonable bounds
            if (name == null) {
                throw new IllegalArgumentException("Name cannot be null");
            }
            // Limit name length to prevent excessive memory usage
            if (name.length() > 255) {
                throw new IllegalArgumentException("Name exceeds maximum length of 255 characters");
            }
            
            this.status = status;
            // Defensive copy to prevent external modification
            this.name = name;
        }
        
        public int getStatus() {
            return status;
        }
        
        public String getName() {
            return name;
        }
        
        // Clear sensitive data if needed
        public void clear() {
            this.status = 0;
            // String is immutable in Java, but we can nullify the reference
            this.name = null;
        }
    }
    
    // Factory method to create a person with validated inputs
    public static Person createPerson(int status, String name) {
        // Validate status is within expected range
        if (status < 0 || status > 1000) {
            throw new IllegalArgumentException("Status must be between 0 and 1000");
        }
        
        // Sanitize name input - remove control characters
        if (name != null) {
            name = name.replaceAll("[\\\\p{Cntrl}]", "");
        }
        
        // Allocate and return new Person object
        return new Person(status, name);
    }
    
    public static void main(String[] args) {
        // Test case 1: Create person with status 0 and name "John"
        try {
            Person person1 = createPerson(0, "John");
            System.out.println("Test 1 - Status: " + person1.getStatus() + ", Name: " + person1.getName());
            person1.clear();
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Create person with different status
        try {
            Person person2 = createPerson(1, "Alice");
            System.out.println("Test 2 - Status: " + person2.getStatus() + ", Name: " + person2.getName());
            person2.clear();
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Create person with empty name
        try {
            Person person3 = createPerson(0, "");
            System.out.println("Test 3 - Status: " + person3.getStatus() + ", Name: '" + person3.getName() + "'");
            person3.clear();
        } catch (Exception e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Attempt to create with null name (should fail)
        try {
            Person person4 = createPerson(0, null);
            System.out.println("Test 4 - Status: " + person4.getStatus() + ", Name: " + person4.getName());
        } catch (Exception e) {
            System.out.println("Test 4 correctly rejected null name: " + e.getMessage());
        }
        
        // Test case 5: Attempt to create with invalid status (should fail)
        try {
            Person person5 = createPerson(-1, "Bob");
            System.out.println("Test 5 - Status: " + person5.getStatus() + ", Name: " + person5.getName());
        } catch (Exception e) {
            System.out.println("Test 5 correctly rejected invalid status: " + e.getMessage());
        }
    }
}
