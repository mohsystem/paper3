public class Lse74 {
    static final class Person {
        int status;
        Person() {
            this.status = 0;
        }
    }

    // Allocates a new Person and sets status to 0
    public static Person allocatePerson() {
        Person p = new Person();
        p.status = 0; // explicit set to fulfill requirement
        return p;
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 1; i <= 5; i++) {
            Person p = allocatePerson();
            if (p == null) {
                System.err.println("Allocation failed");
                System.exit(1);
            }
            System.out.println("Test " + i + ": status=" + p.status);
        }
    }
}