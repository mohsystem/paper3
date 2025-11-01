public class Lse74 {

    static class Person {
        int status;
    }

    /**
     * Allocates a new Person object and sets its status field to 0.
     * @return A new Person object.
     */
    public static Person allocatePerson() {
        // In Java, 'new' allocates memory for an object.
        Person p = new Person();
        // Instance variables of numeric types are default-initialized to 0,
        // but we set it explicitly as per the requirement.
        p.status = 0;
        return p;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        for (int i = 1; i <= 5; i++) {
            Person person = allocatePerson();
            System.out.println("Test Case " + i + ":");
            if (person != null) {
                System.out.println("  Person allocated at: " + person);
                System.out.println("  Person status: " + person.status);
            } else {
                System.out.println("  Allocation failed.");
            }
        }
    }
}