class Lse77 {
    static class Person {
        private int status;
        private String name;

        // Constructor to initialize the Person object
        public Person(int status, String name) {
            this.status = status;
            this.name = name;
        }

        // Getter for status
        public int getStatus() {
            return status;
        }

        // Getter for name
        public String getName() {
            return name;
        }

        @Override
        public String toString() {
            return "Person{status=" + status + ", name='" + name + "'} at memory address: " + Integer.toHexString(System.identityHashCode(this));
        }
    }

    /**
     * Creates a new Person object, which in Java is the equivalent of allocating memory for an object.
     * The new object is initialized with status=0 and name="John".
     *
     * @return A reference to the newly created Person object.
     */
    public static Person createPerson() {
        // The 'new' keyword allocates memory on the heap for the Person object
        // and returns a reference to it.
        Person person = new Person(0, "John");
        return person;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            Person p = createPerson();
            // In Java, memory is managed automatically by the Garbage Collector.
            // We don't need to manually free the memory allocated for 'p'.
            if (p != null) {
                System.out.println("  Successfully created person: " + p.toString());
            } else {
                System.out.println("  Failed to create person.");
            }
        }
    }
}