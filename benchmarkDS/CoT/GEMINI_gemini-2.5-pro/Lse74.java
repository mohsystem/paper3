class Person {
    int status;
    String name;

    // Constructor to initialize the person object
    public Person() {
        // The prompt requires setting the status field to 0 upon allocation.
        this.status = 0;
        this.name = "Default"; // Default value for other fields
    }

    public int getStatus() {
        return this.status;
    }
}

public class Lse74 {

    /**
     * Allocates a new Person object and initializes its status field to 0.
     * In Java, memory allocation is handled by the 'new' keyword, and the
     * constructor handles initialization.
     *
     * @return A new Person object.
     */
    public static Person createPerson() {
        // The 'new' keyword allocates memory, and the Person() constructor
        // is called, which sets the status to 0.
        return new Person();
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        for (int i = 1; i <= 5; i++) {
            System.out.println("Test Case " + i + ":");
            Person p = null;
            try {
                p = createPerson();
                // Check for null is a good practice, though 'new' throws an
                // OutOfMemoryError instead of returning null on failure.
                if (p != null) {
                    System.out.println("  Person created successfully.");
                    System.out.println("  Initial status: " + p.getStatus());
                } else {
                    System.out.println("  Failed to create person.");
                }
            } catch (OutOfMemoryError e) {
                System.err.println("  Test Case " + i + " failed: Out of memory.");
            }
            // In Java, there's no need to manually free memory.
            // The Garbage Collector will handle it.
            System.out.println("--------------------");
        }
    }
}