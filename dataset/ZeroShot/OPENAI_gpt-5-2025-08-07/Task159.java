import java.util.OptionalInt;

public class Task159 {
    private static final int MAX_SIZE = 1_000_000;

    // Dynamically allocate a buffer and initialize with sequential values starting from startVal.
    public static int[] allocateBuffer(int size, int startVal) {
        if (size < 0 || size > MAX_SIZE) {
            return null;
        }
        int[] buffer = new int[size];
        for (int i = 0; i < size; i++) {
            buffer[i] = startVal + i;
        }
        return buffer;
    }

    // Safely read from buffer at the given index. Returns OptionalInt.empty() if invalid.
    public static OptionalInt readAt(int[] buffer, int index) {
        if (buffer == null || index < 0 || index >= buffer.length) {
            return OptionalInt.empty();
        }
        return OptionalInt.of(buffer[index]);
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] buf1 = allocateBuffer(10, 100);
        System.out.println("Test1: " + (readAt(buf1, 0).isPresent() ? readAt(buf1, 0).getAsInt() : "Invalid"));

        int[] buf2 = allocateBuffer(10, 200);
        System.out.println("Test2: " + (readAt(buf2, 9).isPresent() ? readAt(buf2, 9).getAsInt() : "Invalid"));

        int[] buf3 = allocateBuffer(10, 300);
        System.out.println("Test3: " + (readAt(buf3, 10).isPresent() ? readAt(buf3, 10).getAsInt() : "Invalid"));

        int[] buf4 = allocateBuffer(0, 400);
        System.out.println("Test4: " + (readAt(buf4, 0).isPresent() ? readAt(buf4, 0).getAsInt() : "Invalid"));

        int[] buf5 = allocateBuffer(5, 500);
        System.out.println("Test5: " + (readAt(buf5, -1).isPresent() ? readAt(buf5, -1).getAsInt() : "Invalid"));
    }
}