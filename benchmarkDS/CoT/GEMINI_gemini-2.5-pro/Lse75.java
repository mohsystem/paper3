public class Lse75 {

    static class Person {
        int status;
        int id;
        private static int counter = 0;

        /**
         * Constructor for Person. It is called when a new object is created.
         * Initializes the status to 0.
         */
        public Person() {
            // Set status to 0 as required
            this.status = 0;
            this.id = ++counter;
        }

        @Override
        public String toString() {
            return "Person(id=" + id + ", status=" + status + ")";
        }
    }

    /**
     * Allocates memory for a new Person object, initializes its status to 0,
     * and returns a reference to it.
     *
     * @return A reference to the newly created Person object.
     */
    public static Person allocatePerson() {
        // In Java, the 'new' keyword allocates memory and invokes the constructor.
        // The constructor handles the initialization of the status field.
        return new Person();
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        Person[] people = new Person[5];

        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            // The 'new' operator can throw an OutOfMemoryError, but it's an Error,
            // not a checked Exception, and is generally not caught in typical applications.
            people[i] = allocatePerson();
            if (people[i] != null) {
                System.out.println("  Allocated: " + people[i].toString());
                System.out.println("  Person status is: " + people[i].status);
            } else {
                // This branch is unlikely to be hit without catching OutOfMemoryError
                System.out.println("  Memory allocation failed.");
            }
        }
        // In Java, there is no need to manually deallocate memory.
        // The Garbage Collector automatically reclaims memory for objects
        // that are no longer referenced.
        System.out.println("\nMemory will be managed by the Garbage Collector.");
    }
}