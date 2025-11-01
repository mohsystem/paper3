public class Lse78 {

    static class Person {
        private int status;
        private String name;

        /**
         * Constructor to allocate and initialize a Person object.
         * It sets the status to 0 as required.
         *
         * @param name The name of the person.
         * @throws IllegalArgumentException if the name is null or empty.
         */
        public Person(String name) {
            if (name == null || name.isEmpty()) {
                throw new IllegalArgumentException("Name cannot be null or empty.");
            }
            this.name = name;
            this.status = 0; // Set status to 0
        }

        public int getStatus() {
            return status;
        }

        public String getName() {
            return name;
        }

        @Override
        public String toString() {
            return "Person{name='" + name + "', status=" + status + '}';
        }
    }

    /**
     * Allocates a new person and sets the status to 0.
     * In Java, memory allocation is handled by 'new' and the constructor handles initialization.
     * This method serves as a safe factory.
     *
     * @param name The name of the person.
     * @return A new Person object, or null on failure (e.g., invalid name, out of memory).
     */
    public static Person createPerson(String name) {
        try {
            return new Person(name);
        } catch (IllegalArgumentException e) {
            System.err.println("Error creating person: " + e.getMessage());
            return null;
        } catch (OutOfMemoryError e) {
            System.err.println("Error creating person: Out of memory.");
            // In a real application, this might be a fatal error.
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1
        Person person1 = createPerson("Alice");
        if (person1 != null) {
            System.out.println("Test Case 1: Created " + person1);
        }

        // Test Case 2
        Person person2 = createPerson("Bob");
        if (person2 != null) {
            System.out.println("Test Case 2: Created " + person2);
        }

        // Test Case 3
        Person person3 = createPerson("Charlie");
        if (person3 != null) {
            System.out.println("Test Case 3: Created " + person3);
        }

        // Test Case 4
        Person person4 = createPerson("Diana");
        if (person4 != null) {
            System.out.println("Test Case 4: Created " + person4);
        }

        // Test Case 5: Invalid input
        Person person5 = createPerson("");
        if (person5 == null) {
            System.out.println("Test Case 5: Failed to create person as expected for empty name.");
        }

        // In Java, memory is managed by the Garbage Collector. No explicit deallocation is needed.
        System.out.println("\n--- Test Cases Finished ---");
    }
}