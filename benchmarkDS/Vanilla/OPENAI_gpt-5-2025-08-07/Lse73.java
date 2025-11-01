import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public class Lse73 {
    // Simulated "sizeof(person)" layout in Java
    static final int NAME_SIZE = 50;    // char[50]
    static final int AGE_SIZE = 4;      // int
    static final int HEIGHT_SIZE = 8;   // double
    static final int PERSON_SIZE = NAME_SIZE + AGE_SIZE + HEIGHT_SIZE;

    // Allocates a block of memory of size "sizeof(person)" and returns a "pointer"-like buffer
    public static ByteBuffer allocatePersonMemory() {
        return ByteBuffer.allocate(PERSON_SIZE);
    }

    // Helper to write a "person" into the allocated memory block
    public static void writePerson(ByteBuffer buf, String name, int age, double height) {
        buf.clear();
        byte[] nameBytes = name.getBytes(StandardCharsets.UTF_8);
        byte[] fixedName = new byte[NAME_SIZE];
        int len = Math.min(nameBytes.length, NAME_SIZE - 1); // leave room for null terminator
        System.arraycopy(nameBytes, 0, fixedName, 0, len);
        fixedName[len] = 0; // null-terminate to mimic C-style string
        buf.put(fixedName);
        buf.putInt(age);
        buf.putDouble(height);
        buf.flip();
    }

    // Helper to read and print a "person" from the buffer
    public static String readPerson(ByteBuffer buf) {
        buf.rewind();
        byte[] nameBytes = new byte[NAME_SIZE];
        buf.get(nameBytes);
        int end = 0;
        while (end < NAME_SIZE && nameBytes[end] != 0) end++;
        String name = new String(nameBytes, 0, end, StandardCharsets.UTF_8);
        int age = buf.getInt();
        double height = buf.getDouble();
        return "Person{name='" + name + "', age=" + age + ", height=" + height + ", blockSize=" + buf.capacity() + "}";
    }

    public static void main(String[] args) {
        // 5 test cases
        ByteBuffer b1 = allocatePersonMemory();
        writePerson(b1, "Alice", 30, 1.65);
        System.out.println(readPerson(b1));

        ByteBuffer b2 = allocatePersonMemory();
        writePerson(b2, "Bob", 25, 1.80);
        System.out.println(readPerson(b2));

        ByteBuffer b3 = allocatePersonMemory();
        writePerson(b3, "Carol", 40, 1.70);
        System.out.println(readPerson(b3));

        ByteBuffer b4 = allocatePersonMemory();
        writePerson(b4, "Dave", 35, 1.75);
        System.out.println(readPerson(b4));

        ByteBuffer b5 = allocatePersonMemory();
        writePerson(b5, "Eve", 28, 1.60);
        System.out.println(readPerson(b5));
    }
}