public class Lse77 {

    static class Person {
        int status;
        String name;

        @Override
        public String toString() {
            return "Person[name=" + name + ", status=" + status + "]";
        }
    }

    /**
     * Allocates a new Person object. In Java, memory is allocated on the heap
     * via the 'new' keyword, and is automatically managed by the Garbage Collector.
     * @return A reference to the newly created Person object.
     */
    public static Person createPerson() {
        Person person = new Person();
        person.status = 0;
        person.name = "John";
        return person;
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");
        for (int i = 1; i <= 5; i++) {
            System.out.println("Test Case " + i + ":");
            Person p = createPerson();
            // In Java, there is no need to manually deallocate memory.
            // The object 'p' will be garbage-collected when it's no longer reachable.
            if (p != null) {
                System.out.println("  Created: " + p.toString());
                // System.identityHashCode provides a number usually based on the object's memory address
                System.out.println("  Object's identity hash code: " + System.identityHashCode(p));
            } else {
                System.out.println("  Failed to create person object.");
            }
        }
        System.out.println("--- Java Test Cases Complete ---");
    }
}