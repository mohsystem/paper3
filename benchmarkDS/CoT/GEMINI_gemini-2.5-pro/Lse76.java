class Lse76 {
    // Person class to represent the data structure
    static class Person {
        int status;

        // Constructor to initialize the status
        public Person() {
            // By default, an int is initialized to 0 in Java,
            // but we set it explicitly for clarity.
            this.status = 0;
        }

        public int getStatus() {
            return this.status;
        }
    }

    /**
     * Allocates a new Person object and sets its status to 0.
     * In Java, this is done by creating a new instance of the class.
     * The constructor handles the initialization.
     *
     * @return A new Person object.
     */
    public static Person allocatePerson() {
        // The 'new' keyword allocates memory, and the constructor sets the status.
        return new Person();
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases in Java:");
        for (int i = 0; i < 5; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            Person p = allocatePerson();
            if (p != null) {
                System.out.println("Person allocated successfully.");
                System.out.println("Person status: " + p.getStatus());
            } else {
                System.out.println("Failed to allocate Person.");
            }
            // In Java, memory is managed automatically by the Garbage Collector.
            // No explicit free/delete is needed.
        }
    }
}