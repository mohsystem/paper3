public class Lse77 {

    static class Person {
        private int status;
        private String name;

        // Constructor to initialize the Person object
        public Person(int status, String name) {
            this.status = status;
            this.name = name;
        }

        public int getStatus() {
            return status;
        }

        public String getName() {
            return name;
        }

        @Override
        public String toString() {
            return "Person[Name: " + name + ", Status: " + status + "]";
        }
    }

    /**
     * Allocates and initializes a new Person object.
     * In Java, memory allocation is handled by the 'new' keyword and the JVM.
     * @return A new Person object.
     */
    public static Person createPerson() {
        // "Allocates" a new Person object and initializes it.
        return new Person(0, "John");
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            Person person = createPerson();
            if (person != null) {
                System.out.println("  Person created successfully.");
                System.out.println("  " + person.toString());
            } else {
                System.out.println("  Failed to create person.");
            }
            // In Java, memory is managed by the Garbage Collector, so no explicit free is needed.
            System.out.println();
        }
    }
}