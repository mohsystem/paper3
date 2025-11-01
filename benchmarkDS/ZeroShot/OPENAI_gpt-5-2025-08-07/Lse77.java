import java.nio.charset.StandardCharsets;

class Lse77 {

    private static final int MAX_ALLOC_SIZE = 100 * 1024 * 1024; // 100 MiB cap to avoid excessive allocation

    // Secure memory allocation method: returns a zero-initialized byte array or null on failure/invalid size.
    public static byte[] allocateMemory(int size) {
        if (size < 0 || size > MAX_ALLOC_SIZE) {
            return null;
        }
        try {
            return new byte[size]; // zero-initialized
        } catch (OutOfMemoryError e) {
            return null;
        }
    }

    // Person model
    static class Person {
        private int status;
        private String name;

        public Person(int status, String name) {
            this.status = status;
            this.name = name;
        }

        public int getStatus() { return status; }
        public String getName() { return name; }

        @Override
        public String toString() {
            return "Person{name='" + name + "', status=" + status + "}";
        }
    }

    // Creates a new person with status 0 and name "John"
    public static Person createDefaultPerson() {
        return new Person(0, "John");
    }

    // 5 test cases
    public static void main(String[] args) {
        byte[] a1 = allocateMemory(16);
        System.out.println("Test1 allocate 16 -> " + (a1 != null ? a1.length : "null"));

        byte[] a2 = allocateMemory(0);
        System.out.println("Test2 allocate 0 -> " + (a2 != null ? a2.length : "null"));

        byte[] a3 = allocateMemory(1_048_576);
        System.out.println("Test3 allocate 1_048_576 -> " + (a3 != null ? a3.length : "null"));

        byte[] a4 = allocateMemory(-5);
        System.out.println("Test4 allocate -5 -> " + (a4 != null ? a4.length : "null"));

        Person p = createDefaultPerson();
        System.out.println("Test5 create person -> " + p);
    }
}