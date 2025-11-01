import java.util.Objects;

public class Lse75 {

    // Simple Person class with status set to 0 on allocation
    public static final class Person {
        private final String name;
        private final int age;
        private int status;

        private Person(String name, int age) {
            this.name = name;
            this.age = age;
            this.status = 0; // status explicitly set to 0
        }

        public String getName() {
            return name;
        }

        public int getAge() {
            return age;
        }

        public int getStatus() {
            return status;
        }

        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + ", status=" + status + "}";
        }
    }

    // Allocates a new Person and sets status to 0
    public static Person allocatePerson(String name, int age) {
        String safeName = (name == null || name.trim().isEmpty()) ? "unknown" : name.trim();
        if (age < 0 || age > 150) {
            return null; // Fail closed on invalid age
        }
        return new Person(safeName, age);
    }

    public static void main(String[] args) {
        // 5 test cases
        Person p1 = allocatePerson("Alice", 30);
        Person p2 = allocatePerson("Bob", 45);
        Person p3 = allocatePerson("Charlie", 0);
        Person p4 = allocatePerson("Dana", 99);
        Person p5 = allocatePerson(null, 25);

        System.out.println(p1 != null ? p1.toString() : "Allocation failed");
        System.out.println(p2 != null ? p2.toString() : "Allocation failed");
        System.out.println(p3 != null ? p3.toString() : "Allocation failed");
        System.out.println(p4 != null ? p4.toString() : "Allocation failed");
        System.out.println(p5 != null ? p5.toString() : "Allocation failed");
    }
}