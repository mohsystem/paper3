
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public class Lse73 {
    // Person class to represent the data structure
    static class Person {
        private String name;
        private int age;
        private String address;
        
        public Person() {
            this.name = "";
            this.age = 0;
            this.address = "";
        }
        
        public Person(String name, int age, String address) {
            // Input validation: check for null and enforce length limits
            if (name == null || name.length() > 100) {
                throw new IllegalArgumentException("Invalid name");
            }
            if (age < 0 || age > 150) {
                throw new IllegalArgumentException("Invalid age");
            }
            if (address == null || address.length() > 200) {
                throw new IllegalArgumentException("Invalid address");
            }
            this.name = name;
            this.age = age;
            this.address = address;
        }
        
        public String getName() { return name; }
        public int getAge() { return age; }
        public String getAddress() { return address; }
        
        public void setName(String name) {
            if (name == null || name.length() > 100) {
                throw new IllegalArgumentException("Invalid name");
            }
            this.name = name;
        }
        
        public void setAge(int age) {
            if (age < 0 || age > 150) {
                throw new IllegalArgumentException("Invalid age");
            }
            this.age = age;
        }
        
        public void setAddress(String address) {
            if (address == null || address.length() > 200) {
                throw new IllegalArgumentException("Invalid address");
            }
            this.address = address;
        }
    }
    
    // Memory allocation method that allocates memory for a Person object
    // Returns a newly allocated Person object (Java handles memory automatically)
    public static Person allocatePerson() {
        // Java automatically handles memory allocation through 'new' operator
        // Memory is managed by the JVM and garbage collector
        Person person = new Person();
        if (person == null) {
            throw new OutOfMemoryError("Failed to allocate memory for Person");
        }
        return person;
    }
    
    // Overloaded method to allocate and initialize Person
    public static Person allocatePerson(String name, int age, String address) {
        // Validate inputs before allocation
        if (name == null || name.length() > 100) {
            throw new IllegalArgumentException("Invalid name");
        }
        if (age < 0 || age > 150) {
            throw new IllegalArgumentException("Invalid age");
        }
        if (address == null || address.length() > 200) {
            throw new IllegalArgumentException("Invalid address");
        }
        
        Person person = new Person(name, age, address);
        if (person == null) {
            throw new OutOfMemoryError("Failed to allocate memory for Person");
        }
        return person;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic allocation
        System.out.println("Test 1: Basic allocation");
        Person p1 = allocatePerson();
        System.out.println("Person allocated: " + (p1 != null ? "Success" : "Failed"));
        
        // Test case 2: Allocation with initialization
        System.out.println("\\nTest 2: Allocation with initialization");
        Person p2 = allocatePerson("John Doe", 30, "123 Main St");
        System.out.println("Name: " + p2.getName() + ", Age: " + p2.getAge() + ", Address: " + p2.getAddress());
        
        // Test case 3: Multiple allocations
        System.out.println("\\nTest 3: Multiple allocations");
        Person p3 = allocatePerson("Alice Smith", 25, "456 Oak Ave");
        Person p4 = allocatePerson("Bob Johnson", 45, "789 Pine Rd");
        System.out.println("p3 - Name: " + p3.getName() + ", Age: " + p3.getAge());
        System.out.println("p4 - Name: " + p4.getName() + ", Age: " + p4.getAge());
        
        // Test case 4: Invalid input handling
        System.out.println("\\nTest 4: Invalid input handling");
        try {
            Person p5 = allocatePerson(null, 30, "Address");
            System.out.println("Should not reach here");
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        
        // Test case 5: Boundary value testing
        System.out.println("\\nTest 5: Boundary value testing");
        try {
            Person p6 = allocatePerson("Valid Name", -5, "Address");
            System.out.println("Should not reach here");
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception for invalid age: " + e.getMessage());
        }
        Person p7 = allocatePerson("Edge Case", 0, "Valid Address");
        System.out.println("Allocated person with age 0: " + (p7 != null ? "Success" : "Failed"));
    }
}
