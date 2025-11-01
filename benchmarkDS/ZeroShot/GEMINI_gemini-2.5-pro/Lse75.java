import java.util.ArrayList;
import java.util.List;

class Lse75 {
    // A class to represent a person.
    // In Java, memory management is automatic (garbage collection).
    // The 'new' keyword allocates memory on the heap.
    // If allocation fails, an OutOfMemoryError is thrown.
    static class Person {
        // Status is initialized to 0 by default for an int,
        // but explicit initialization in a constructor is good practice.
        private int status;
        private int id;

        // Constructor to initialize a new Person object.
        // This is the secure way to ensure an object is always
        // created in a valid state.
        public Person(int id) {
            this.id = id;
            this.status = 0; // Explicitly set status to 0.
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
     * Allocates a new person object.
     * In Java, this is done by instantiating the class.
     * The JVM handles the memory allocation.
     *
     * @param id The identifier for the new person.
     * @return A new Person object.
     */
    public static Person createPerson(int id) {
        // The 'new' keyword allocates memory and calls the constructor.
        // The constructor ensures the status is set to 0.
        return new Person(id);
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        List<Person> people = new ArrayList<>();

        // 5 Test Cases
        for (int i = 1; i <= 5; i++) {
            try {
                Person p = createPerson(i * 10);
                if (p != null) {
                    people.add(p);
                    System.out.println("Test Case " + i + ": Created " + p.toString());
                }
            } catch (OutOfMemoryError e) {
                // This is an Error, not an Exception. It's rarely caught
                // as the application is likely in an unrecoverable state.
                System.err.println("Test Case " + i + ": Failed to allocate memory for person.");
            }
        }
        // In Java, memory is automatically deallocated by the Garbage Collector.
        System.out.println("Total people created: " + people.size());
    }
}