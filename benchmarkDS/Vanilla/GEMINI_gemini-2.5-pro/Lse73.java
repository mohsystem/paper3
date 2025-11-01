public class Lse73 {

    static class Person {
        String name;
        int age;

        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + "}";
        }
    }

    /**
     * Allocates memory for a Person object. In Java, this is done by creating a new instance.
     *
     * @return A reference to the newly created Person object.
     */
    public static Person allocatePerson() {
        return new Person();
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");

        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            Person p = allocatePerson();
            if (p != null) {
                // Initialize the allocated object
                p.name = "Person " + i;
                p.age = 20 + i;
                System.out.println("Successfully allocated and initialized: " + p);
                // In Java, memory is automatically managed by the Garbage Collector.
                // No need to explicitly deallocate.
            } else {
                System.out.println("Memory allocation failed.");
            }
        }
    }
}