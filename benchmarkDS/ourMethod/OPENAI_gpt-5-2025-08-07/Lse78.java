import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse78 {

    private static final int MAX_ALLOC = 64 * 1024 * 1024; // 64MB cap
    private static final int MAX_NAME_LEN = 128;

    public static final class Person {
        private final String name;
        private final int age;
        private final int status;

        public Person(String name, int age, int status) {
            this.name = name;
            this.age = age;
            this.status = status;
        }

        public String name() { return name; }
        public int age() { return age; }
        public int status() { return status; }

        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + ", status=" + status + "}";
        }
    }

    // Memory allocation: returns a new byte array if size is valid
    public static byte[] allocateMemory(int size) {
        if (size < 0 || size > MAX_ALLOC) {
            throw new IllegalArgumentException("invalid size");
        }
        return new byte[size];
    }

    // Allocates a new Person and sets status to 0
    public static Person allocatePerson(String name, int age) {
        if (name == null) {
            throw new IllegalArgumentException("name null");
        }
        if (age < 0 || age > 150) {
            throw new IllegalArgumentException("invalid age");
        }
        String trimmed = name.trim();
        if (trimmed.length() > MAX_NAME_LEN) {
            trimmed = trimmed.substring(0, MAX_NAME_LEN);
        }
        return new Person(trimmed, age, 0);
    }

    // Helper: safely preview memory buffer for demo
    private static String preview(byte[] buf) {
        int n = Math.min(buf.length, 8);
        byte[] head = Arrays.copyOf(buf, n);
        return "len=" + buf.length + ", head=" + Arrays.toString(head);
    }

    public static void main(String[] args) {
        // 5 test cases: each creates a person and allocates memory
        String[] names = {
            "Alice",
            "Bob",
            "Charlie",
            "ThisIsAVeryLongNameThatWillBeClippedBecauseItExceedsTheMaximumAllowedLengthABCDEFGHIJKLMNOPQRSTUVWXYZ",
            "  Eve  "
        };
        int[] ages = {30, 25, 40, 22, 28};
        int[] sizes = {16, 0, 1024, MAX_ALLOC, MAX_ALLOC + 1};

        for (int i = 0; i < 5; i++) {
            try {
                Person p = allocatePerson(names[i], ages[i]);
                System.out.println("Created: " + p);
            } catch (Exception e) {
                System.out.println("Person alloc failed: " + e.getMessage());
            }

            try {
                byte[] mem = allocateMemory(sizes[i]);
                // Fill a couple bytes for demonstration (bounded)
                if (mem.length >= 2) {
                    mem[0] = (byte) i;
                    mem[1] = (byte) (i * 2);
                }
                System.out.println("Memory: " + preview(mem));
            } catch (Exception e) {
                System.out.println("Memory alloc failed: " + e.getMessage());
            }
        }
    }
}