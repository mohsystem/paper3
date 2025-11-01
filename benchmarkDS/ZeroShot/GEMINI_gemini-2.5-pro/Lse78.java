import java.util.ArrayList;
import java.util.List;

public class Lse78 {

    static class Person {
        // In Java, integer instance variables are automatically initialized to 0.
        // We will also explicitly initialize it in the constructor for clarity.
        private int status;
        private int id; // Added for testing purposes

        public Person(int id) {
            this.id = id;
            this.status = 0; // Explicitly set status to 0 as per requirement
        }

        public int getStatus() {
            return status;
        }

        public int getId() {
            return id;
        }

        @Override
        public String toString() {
            return "Person{id=" + id + ", status=" + status + "}";
        }
    }

    /**
     * Allocates a new Person object and sets its status to 0.
     * In Java, memory allocation is handled by the 'new' keyword and garbage collection.
     * The JVM throws an OutOfMemoryError if allocation fails, which is a non-recoverable
     * error for most applications.
     *
     * @param id The identifier for the new person.
     * @return A reference to the newly created Person object.
     */
    public static Person allocatePerson(int id) {
        // 'new' allocates memory and the constructor initializes the object.
        return new Person(id);
    }

    public static void main(String[] args) {
        // List to hold the created objects to prevent premature garbage collection
        // in more complex scenarios, and to manage them for the test.
        List<Person> people = new ArrayList<>();
        System.out.println("Running 5 test cases...");

        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            try {
                Person p = allocatePerson(i);
                // In Java, 'new' returns a valid reference or throws an error.
                // So, we don't check for null in the same way as C/C++.
                // The 'p' variable holds a reference to the allocated object.
                people.add(p);
                System.out.println("Successfully allocated: " + p.toString());
            } catch (OutOfMemoryError e) {
                // This is the secure way to handle allocation failure in Java,
                // although catching it is rare unless you have a specific recovery strategy.
                System.err.println("Memory allocation failed for person with id " + i);
            }
        }
        
        // In Java, memory is automatically deallocated by the Garbage Collector.
        // No manual 'free' or 'delete' is required.
        System.out.println("\nProgram finished. Memory will be reclaimed by the garbage collector.");
    }
}