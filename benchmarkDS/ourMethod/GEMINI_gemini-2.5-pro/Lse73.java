public class Lse73 {

    static class Person {
        private String name;
        private int age;

        public Person(String name, int age) {
            // Basic input validation
            if (name == null || name.trim().isEmpty()) {
                throw new IllegalArgumentException("Name cannot be null or empty.");
            }
            if (age < 0) {
                throw new IllegalArgumentException("Age cannot be negative.");
            }
            this.name = name;
            this.age = age;
        }

        @Override
        public String toString() {
            return "Person{" +
                    "name='" + name + '\'' +
                    ", age=" + age +
                    '}';
        }
    }

    /**
     * Allocates memory for a Person object and returns a reference to it.
     * In Java, memory allocation is handled by the 'new' keyword and the JVM.
     * This is the idiomatic way to "allocate a block of memory of size sizeof(person)".
     *
     * @param name The name of the person.
     * @param age  The age of the person.
     * @return A reference to the newly created Person object.
     */
    public static Person allocatePerson(String name, int age) {
        // 'new' allocates memory on the heap for the object and calls the constructor.
        return new Person(name, age);
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");
        // Using an array to hold references to the created objects.
        Person[] people = new Person[5];

        try {
            // Test Case 1
            people[0] = allocatePerson("Alice", 30);
            System.out.println("Test Case 1: Allocated " + people[0]);

            // Test Case 2
            people[1] = allocatePerson("Bob", 25);
            System.out.println("Test Case 2: Allocated " + people[1]);

            // Test Case 3
            people[2] = allocatePerson("Charlie", 35);
            System.out.println("Test Case 3: Allocated " + people[2]);

            // Test Case 4
            people[3] = allocatePerson("Diana", 28);
            System.out.println("Test Case 4: Allocated " + people[3]);

            // Test Case 5
            people[4] = allocatePerson("Eve", 40);
            System.out.println("Test Case 5: Allocated " + people[4]);

            // In Java, memory is managed by the Garbage Collector (GC).
            // There is no need to explicitly free the memory.
            // When the 'people' array and its elements are no longer referenced,
            // the GC will eventually reclaim the memory.

        } catch (OutOfMemoryError e) {
            System.err.println("FATAL: Failed to allocate memory: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            System.err.println("Error: Invalid argument provided for person: " + e.getMessage());
        } catch (Exception e) {
            // Catching general exceptions as a fallback.
            System.err.println("An unexpected error occurred: " + e.getMessage());
        }
        System.out.println("--- Java Test Cases Complete ---");
    }
}