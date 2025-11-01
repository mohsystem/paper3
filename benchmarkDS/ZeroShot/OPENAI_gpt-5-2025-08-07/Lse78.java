import java.util.Arrays;

public class Lse78 {
    private static final int MAX_SAFE_SIZE = 16 * 1024 * 1024; // 16 MB cap

    // Secure memory allocation: returns a zeroed byte array or null on invalid size/OOM
    public static byte[] allocateMemory(int size) {
        if (size < 0 || size > MAX_SAFE_SIZE) {
            return null;
        }
        try {
            byte[] block = new byte[size]; // Java zero-initializes arrays
            return block;
        } catch (OutOfMemoryError e) {
            return null;
        }
    }

    // Allocates a new Person with status set to 0
    public static Person allocatePerson() {
        return new Person();
    }

    public static void main(String[] args) {
        // 5 test cases
        // Test 1: Allocate 1024 bytes
        byte[] t1 = allocateMemory(1024);
        System.out.println("Test1 allocateMemory(1024): " + (t1 != null ? t1.length : "null"));

        // Test 2: Allocate a Person and confirm status == 0
        Person p1 = allocatePerson();
        System.out.println("Test2 allocatePerson(): status=" + (p1 != null ? p1.getStatus() : "null"));

        // Test 3: Allocate 0 bytes
        byte[] t3 = allocateMemory(0);
        System.out.println("Test3 allocateMemory(0): " + (t3 != null ? t3.length : "null"));

        // Test 4: Invalid negative size
        byte[] t4 = allocateMemory(-1);
        System.out.println("Test4 allocateMemory(-1): " + (t4 != null ? t4.length : "null"));

        // Test 5: Excessively large size (exceeds MAX_SAFE_SIZE)
        byte[] t5 = allocateMemory(MAX_SAFE_SIZE + 1);
        System.out.println("Test5 allocateMemory(>MAX): " + (t5 != null ? t5.length : "null"));
    }

    // Person class with status defaulting to 0
    public static final class Person {
        private final int status;

        public Person() {
            this.status = 0;
        }

        public int getStatus() {
            return status;
        }
    }
}