public class Lse75 {
    static class Person {
        public int status;
        public Person(int status) { this.status = status; }
        @Override
        public String toString() { return "Person{status=" + status + "}"; }
    }

    // Allocates a new Person and sets status to 0
    public static Person allocatePerson() {
        return new Person(0);
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 0; i < 5; i++) {
            Person p = allocatePerson();
            System.out.println(p);
        }
    }
}