public class Lse77 {
    private static final int MAX_ALLOC = 1024 * 1024; // 1 MiB cap
    private static final int MAX_NAME_LEN = 256;

    // Memory allocation method: returns a block (byte[]) or null on failure.
    public static byte[] allocateMemory(int size) {
        if (size <= 0 || size > MAX_ALLOC) {
            return null;
        }
        try {
            return new byte[size];
        } catch (OutOfMemoryError e) {
            return null;
        }
    }

    // Creates a new person with given name and status (name is truncated to MAX_NAME_LEN).
    public static Person createPerson(String name, int status) {
        if (name == null) {
            name = "John";
        }
        if (name.length() > MAX_NAME_LEN) {
            name = name.substring(0, MAX_NAME_LEN);
        }
        return new Person(status, name);
    }

    public static final class Person {
        public final int status;
        public final String name;

        public Person(int status, String name) {
            this.status = status;
            this.name = name;
        }

        @Override
        public String toString() {
            return "Person{name='" + name + "', status=" + status + "}";
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        byte[] a1 = allocateMemory(16);
        System.out.println(a1 != null ? "alloc 16 ok: " + a1.length : "alloc 16 failed");

        byte[] a2 = allocateMemory(0);
        System.out.println(a2 != null ? "alloc 0 ok: " + a2.length : "alloc 0 failed");

        byte[] a3 = allocateMemory(MAX_ALLOC);
        System.out.println(a3 != null ? "alloc MAX ok: " + a3.length : "alloc MAX failed");

        byte[] a4 = allocateMemory(MAX_ALLOC + 1);
        System.out.println(a4 != null ? "alloc MAX+1 ok: " + a4.length : "alloc MAX+1 failed");

        Person p = createPerson("John", 0);
        System.out.println("person: " + p);
    }
}