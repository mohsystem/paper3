public class Lse75 {
    public static final class Person {
        public int status;
        private Person() {}
    }

    // Allocates a new Person and sets status to 0.
    public static Person allocatePerson() {
        Person p = new Person();
        p.status = 0;
        return p;
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 1; i <= 5; i++) {
            Person p = allocatePerson();
            System.out.println("Test " + i + ": status=" + p.status);
        }
    }
}