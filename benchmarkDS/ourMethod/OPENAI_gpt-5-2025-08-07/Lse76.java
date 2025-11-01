import java.nio.charset.StandardCharsets;

public class Lse76 {
    private static final int MAX_NAME_LEN = 64;

    public static final class Person {
        private final String name;
        private final int age;
        private final int status; // 0 by allocation requirement

        private Person(String name, int age, int status) {
            this.name = name;
            this.age = age;
            this.status = status;
        }

        public String getName() { return name; }
        public int getAge() { return age; }
        public int getStatus() { return status; }

        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + ", status=" + status + "}";
        }
    }

    private static String sanitizeName(String name) {
        String n = (name == null) ? "" : name.trim();
        // limit by characters, not bytes
        if (n.length() > MAX_NAME_LEN) {
            n = n.substring(0, MAX_NAME_LEN);
        }
        // Ensure encodable in UTF-8 (safe default in Java)
        byte[] b = n.getBytes(StandardCharsets.UTF_8);
        // No expansion risk here; returning the string as-is is fine
        return n;
    }

    private static int sanitizeAge(int age) {
        if (age < 0) return 0;
        if (age > 150) return 150;
        return age;
    }

    // Allocates a new Person and sets status to 0
    public static Person allocatePerson(String name, int age) {
        String safeName = sanitizeName(name);
        int safeAge = sanitizeAge(age);
        return new Person(safeName, safeAge, 0);
    }

    public static void main(String[] args) {
        // 5 test cases
        Person p1 = allocatePerson("Alice", 30);
        Person p2 = allocatePerson(" Bob  ", -5);
        Person p3 = allocatePerson(null, 151);
        Person p4 = allocatePerson("ThisNameIsWayTooLongAndShouldBeTruncatedAtSixtyFourCharacters_ABCDEFGHIJKLMNOPQRSTUVWXYZ", 42);
        Person p5 = allocatePerson("   ", 0);

        System.out.println(p1);
        System.out.println(p2);
        System.out.println(p3);
        System.out.println(p4);
        System.out.println(p5);
    }
}