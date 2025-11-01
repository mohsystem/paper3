import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;

public class Lse73 {

    // Person "struct" layout (C-like)
    // int id (4 bytes) at offset 0
    // char name[32] (32 bytes) at offset 4
    // 4 bytes padding to align double to 8-byte boundary (offset 36..39)
    // double salary (8 bytes) at offset 40
    // total size = 48 bytes
    private static final int OFF_ID = 0;
    private static final int NAME_LEN = 32;
    private static final int OFF_NAME = 4;
    private static final int OFF_SALARY = 40;
    private static final int SIZE = 48;

    // Allocates a block of memory of size sizeof(person) and returns a "pointer-like" buffer
    public static ByteBuffer allocatePersonBlock() {
        ByteBuffer buf = ByteBuffer.allocateDirect(SIZE).order(ByteOrder.nativeOrder());
        zero(buf);
        return buf;
    }

    private static void zero(ByteBuffer buf) {
        for (int i = 0; i < buf.capacity(); i++) {
            buf.put(i, (byte) 0);
        }
    }

    // Helper methods to operate on the allocated block safely
    public static void putId(ByteBuffer buf, int id) {
        validate(buf);
        buf.putInt(OFF_ID, id);
    }

    public static int getId(ByteBuffer buf) {
        validate(buf);
        return buf.getInt(OFF_ID);
    }

    public static void putName(ByteBuffer buf, String name) {
        validate(buf);
        byte[] raw = (name == null) ? new byte[0] : name.getBytes(StandardCharsets.UTF_8);
        byte[] arr = new byte[NAME_LEN];
        int copyLen = Math.min(raw.length, NAME_LEN - 1); // reserve space for NUL
        if (copyLen > 0) {
            System.arraycopy(raw, 0, arr, 0, copyLen);
        }
        for (int i = 0; i < NAME_LEN; i++) {
            buf.put(OFF_NAME + i, arr[i]);
        }
    }

    public static String getName(ByteBuffer buf) {
        validate(buf);
        byte[] arr = new byte[NAME_LEN];
        for (int i = 0; i < NAME_LEN; i++) {
            arr[i] = buf.get(OFF_NAME + i);
        }
        int end = 0;
        while (end < NAME_LEN && arr[end] != 0) {
            end++;
        }
        return new String(arr, 0, end, StandardCharsets.UTF_8);
    }

    public static void putSalary(ByteBuffer buf, double salary) {
        validate(buf);
        buf.putDouble(OFF_SALARY, salary);
    }

    public static double getSalary(ByteBuffer buf) {
        validate(buf);
        return buf.getDouble(OFF_SALARY);
    }

    public static int sizeofPerson() {
        return SIZE;
    }

    private static void validate(ByteBuffer buf) {
        if (buf == null || buf.capacity() < SIZE) {
            throw new IllegalArgumentException("Invalid buffer");
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] names = { "Alice", "Bob", "Carlos", "Diana", "Eve-LongName-ShouldBeTruncated" };
        int[] ids = { 1, 2, 3, 4, 5 };
        double[] salaries = { 50000.0, 60000.5, 72000.75, 81000.0, 90500.25 };

        System.out.println("sizeof(person) = " + sizeofPerson() + " bytes");

        for (int i = 0; i < 5; i++) {
            ByteBuffer person = allocatePersonBlock();
            putId(person, ids[i]);
            putName(person, names[i]);
            putSalary(person, salaries[i]);

            System.out.println("Test " + (i + 1) + ": id=" + getId(person)
                    + ", name=\"" + getName(person) + "\""
                    + ", salary=" + getSalary(person));
        }
    }
}