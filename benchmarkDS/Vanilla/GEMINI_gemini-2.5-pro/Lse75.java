class Person {
    int status;
    int id;

    public Person(int id) {
        this.id = id;
        this.status = 0; // Set status to 0 on creation
    }

    @Override
    public String toString() {
        return "Person(id=" + this.id + ", status=" + this.status + ")";
    }
}

public class Lse75 {
    /**
     * Allocates a new person object. In Java, this is done using the 'new' keyword.
     * The person's status is set to 0 by its constructor.
     * @return A reference to the newly created Person object.
     */
    public static Person allocatePerson(int id) {
        return new Person(id);
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        Person[] people = new Person[5];

        // 5 Test cases
        for (int i = 0; i < 5; i++) {
            people[i] = allocatePerson(i + 1);
            System.out.println("Test Case " + (i + 1) + ": Allocated " + people[i]
                             + " at reference " + System.identityHashCode(people[i]));
        }

        // In Java, memory is automatically managed by the Garbage Collector.
        // There is no need to manually deallocate memory.
    }
}