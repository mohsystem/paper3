public class Lse76 {

    /**
     * A simple class to represent a person.
     */
    static class Person {
        // 0 indicates available, other numbers could represent different states.
        public int status;

        /**
         * Constructor for Person.
         * By default, a new Person's status is set to 0.
         */
        public Person() {
            this.status = 0;
        }

        @Override
        public String toString() {
            return "Person{status=" + status + "}";
        }
    }

    /**
     * Allocates memory for a new Person object and initializes its status to 0.
     * In Java, memory allocation is handled by the 'new' keyword, and the
     * constructor handles initialization.
     *
     * @return a reference to the newly created Person object.
     */
    public static Person allocatePerson() {
        // The 'new' operator allocates memory and calls the constructor.
        return new Person();
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");

        // Test cases: Allocate 5 Person objects
        for (int i = 1; i <= 5; i++) {
            System.out.println("Test Case #" + i);
            try {
                Person p = allocatePerson();
                
                // In Java, 'new' throws OutOfMemoryError on failure, so an explicit
                // null check is not the standard pattern for allocation failure.
                // If we reach here, allocation was successful.
                System.out.println("  Allocation successful: " + p);
                System.out.println("  Status is correctly set to: " + p.status);
                
                // Memory is managed by the Garbage Collector, no explicit free is needed.
            } catch (OutOfMemoryError e) {
                System.err.println("  Memory allocation failed: " + e.getMessage());
                // In a real application, you might try to recover or exit gracefully.
            }
            System.out.println();
        }
    }
}