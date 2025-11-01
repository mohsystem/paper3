public class Lse73 {

    // A simple class to represent a person
    static class Person {
        int age;
        float height;

        // Default constructor
        public Person() {
            this.age = 0;
            this.height = 0.0f;
        }

        @Override
        public String toString() {
            return "Person{age=" + age + ", height=" + height + "}";
        }
    }

    /**
     * Allocates memory for a new Person object.
     * In Java, memory allocation is handled by the 'new' keyword and the JVM.
     * This method is equivalent to allocating sizeof(Person) and returning a reference.
     *
     * @return A reference to a new Person object.
     */
    public static Person allocatePerson() {
        return new Person();
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        for (int i = 1; i <= 5; i++) {
            System.out.println("Test Case " + i + ":");
            
            // 1. Allocate memory for a Person object
            Person p = allocatePerson();

            // In Java, 'new' throws OutOfMemoryError on failure, so p will not be null here
            // unless the error is caught, which is uncommon for this error type.
            
            // 2. Initialize the object's fields
            p.age = 20 + i;
            p.height = 170.5f + i;

            // 3. Print the object's state
            System.out.println("  Allocated and initialized: " + p);
            
            // 4. Memory is automatically managed and will be deallocated by the Garbage Collector
            // when the object is no longer reachable.
            System.out.println("  Memory will be deallocated automatically by GC.");
            System.out.println();
        }
    }
}