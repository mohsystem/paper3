
class Lse73 {
    static class Person {
        String name;
        int age;
        
        public Person() {
            this.name = "";
            this.age = 0;
        }
        
        public Person(String name, int age) {
            this.name = name;
            this.age = age;
        }
        
        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + "}";
        }
    }
    
    public static Person allocatePerson() {
        try {
            Person p = new Person();
            return p;
        } catch (OutOfMemoryError e) {
            System.err.println("Memory allocation failed: " + e.getMessage());
            return null;
        }
    }
    
    public static Person allocatePerson(String name, int age) {
        try {
            Person p = new Person(name, age);
            return p;
        } catch (OutOfMemoryError e) {
            System.err.println("Memory allocation failed: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic allocation
        Person p1 = allocatePerson();
        if (p1 != null) {
            p1.name = "Alice";
            p1.age = 25;
            System.out.println("Test 1: " + p1);
        }
        
        // Test case 2: Allocation with parameters
        Person p2 = allocatePerson("Bob", 30);
        if (p2 != null) {
            System.out.println("Test 2: " + p2);
        }
        
        // Test case 3: Multiple allocations
        Person p3 = allocatePerson("Charlie", 35);
        if (p3 != null) {
            System.out.println("Test 3: " + p3);
        }
        
        // Test case 4: Null validation
        Person p4 = allocatePerson("Diana", 28);
        if (p4 != null) {
            System.out.println("Test 4: " + p4);
        } else {
            System.out.println("Test 4: Allocation failed");
        }
        
        // Test case 5: Zero age
        Person p5 = allocatePerson("Eve", 0);
        if (p5 != null) {
            System.out.println("Test 5: " + p5);
        }
    }
}
