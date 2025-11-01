// Chain-of-Through secure implementation in Java
// 1) Problem: Provide a safe memory allocation method and create a Person with status=0 and name="John".
// 2) Security: Validate size, cap maximum allocation, handle OOM, avoid negative sizes.
// 3) Secure coding: Defensive checks and safe defaults.
// 4) Review: Line-by-line checks to ensure no unsafe operations.
// 5) Output: Final secure code.

import java.util.Arrays;

public class Lse77 {

    // Cap allocations to mitigate memory abuse (16 MB).
    private static final int MAX_ALLOC = 16 * 1024 * 1024;

    public static byte[] secureAllocate(int size) {
        if (size <= 0 || size > MAX_ALLOC) {
            return null;
        }
        try {
            // Java arrays are zero-initialized by default.
            return new byte[size];
        } catch (OutOfMemoryError e) {
            return null;
        }
    }

    public static class Person {
        public int status;
        public String name;

        @Override
        public String toString() {
            return "Person{name='" + name + "', status=" + status + "}";
        }
    }

    public static Person createPerson() {
        Person p = new Person();
        // Secure default initialization
        p.status = 0;
        p.name = "John";
        return p;
    }

    public static void main(String[] args) {
        // 5 test cases for memory allocation
        int[] sizes = {0, 16, 1024, -5, 20_000_000};
        for (int s : sizes) {
            byte[] mem = secureAllocate(s);
            System.out.println("secureAllocate(" + s + ") -> " + (mem == null ? "null" : ("byte[" + mem.length + "]")));
        }

        // 5 test cases for person creation
        for (int i = 0; i < 5; i++) {
            Person p = createPerson();
            System.out.println("Test " + (i + 1) + " createPerson -> " + p);
        }
    }
}