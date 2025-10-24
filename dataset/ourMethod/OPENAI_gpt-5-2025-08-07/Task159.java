import java.util.Arrays;

public class Task159 {
    private static final int MAX_BUFFER_SIZE = 1_048_576;

    public static final class ReadResult {
        public final boolean success;
        public final int value;

        public ReadResult(boolean success, int value) {
            this.success = success;
            this.value = value;
        }
    }

    public static byte[] allocateBuffer(int size) {
        if (size < 0 || size > MAX_BUFFER_SIZE) {
            throw new IllegalArgumentException("Invalid buffer size");
        }
        return new byte[size];
    }

    public static void fillSequential(byte[] buffer) {
        if (buffer == null) {
            throw new IllegalArgumentException("Buffer is null");
        }
        for (int i = 0; i < buffer.length; i++) {
            buffer[i] = (byte) (i & 0xFF);
        }
    }

    public static ReadResult safeRead(byte[] buffer, int index) {
        if (buffer == null) {
            return new ReadResult(false, 0);
        }
        if (index < 0 || index >= buffer.length) {
            return new ReadResult(false, 0);
        }
        int value = buffer[index] & 0xFF;
        return new ReadResult(true, value);
    }

    public static void main(String[] args) {
        byte[] buffer = allocateBuffer(10);
        fillSequential(buffer);

        int[] testIndices = new int[] { -1, 0, 5, 9, 10 };
        for (int idx : testIndices) {
            ReadResult res = safeRead(buffer, idx);
            if (res.success) {
                System.out.println("Index " + idx + " -> value: " + res.value);
            } else {
                System.out.println("Index " + idx + " -> error: out of bounds");
            }
        }

        // Additional checks to ensure correctness and safety
        // Not strictly required, but demonstrates the code path remains consistent.
        System.out.println("Buffer length: " + buffer.length);
        System.out.println("Buffer content: " + Arrays.toString(buffer));
    }
}