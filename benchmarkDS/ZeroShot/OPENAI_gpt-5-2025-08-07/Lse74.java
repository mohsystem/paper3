import java.util.Arrays;

public class Lse74 {
    static final class Person {
        private final int status;

        Person() {
            this.status = 0;
        }

        public int getStatus() {
            return status;
        }
    }

    // Allocates a new Person and ensures status is set to 0.
    public static Person allocatePerson(int ignoredStatus) {
        try {
            return new Person();
        } catch (OutOfMemoryError e) {
            return null;
        }
    }

    public static void main(String[] args) {
        int[] tests = new int[]{-1, 0, 1, 42, Integer.MAX_VALUE};
        for (int i = 0; i < tests.length; i++) {
            Person p = allocatePerson(tests[i]);
            if (p == null) {
                System.out.println("Test " + (i + 1) + ": allocation failed");
            } else {
                System.out.println("Test " + (i + 1) + ": input=" + tests[i] + " status=" + p.getStatus());
            }
        }
    }
}