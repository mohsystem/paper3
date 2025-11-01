class Lse77 {
    
    static class Person {
        int status;
        String name;

        // Constructor to make object creation easier
        public Person(int status, String name) {
            this.status = status;
            this.name = name;
        }

        @Override
        public String toString() {
            return "Person{name='" + name + "', status=" + status + "}";
        }
    }

    /**
     * Allocates memory for a new Person object and initializes its fields.
     * In Java, memory allocation is handled by the 'new' keyword.
     * @return A new Person object with name "John" and status 0.
     */
    public static Person createPerson() {
        // Creates a new person and sets the status to 0 and the name to "John".
        Person p = new Person(0, "John");
        return p;
    }

    public static void main(String[] args) {
        System.out.println("Running Java Test Cases:");
        
        // Test Case 1
        Person p1 = createPerson();
        System.out.println("Test Case 1: " + p1);

        // Test Case 2
        Person p2 = createPerson();
        System.out.println("Test Case 2: " + p2);

        // Test Case 3
        Person p3 = createPerson();
        System.out.println("Test Case 3: " + p3);

        // Test Case 4
        Person p4 = createPerson();
        System.out.println("Test Case 4: " + p4);

        // Test Case 5
        Person p5 = createPerson();
        System.out.println("Test Case 5: " + p5);
        
        // In Java, the Garbage Collector handles freeing memory, so no manual deallocation is needed.
    }
}