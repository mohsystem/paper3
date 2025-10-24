// Chain-of-Through process in code generation applied with secure checks.
// Language: Java

public class Task159 {

    // Result container to avoid throwing exceptions for normal invalid input cases.
    public static final class ReadResult {
        public final boolean success;
        public final byte value;

        private ReadResult(boolean success, byte value) {
            this.success = success;
            this.value = value;
        }

        public static ReadResult ok(byte value) {
            return new ReadResult(true, value);
        }

        public static ReadResult fail() {
            return new ReadResult(false, (byte) 0);
        }
    }

    private static final int MAX_ALLOC = 10_000_000; // Prevent excessive allocation

    // Secure allocation with bounds checks
    public static byte[] allocateBuffer(int size) {
        if (size < 0 || size > MAX_ALLOC) {
            throw new IllegalArgumentException("Invalid buffer size");
        }
        byte[] buf = new byte[size];
        for (int i = 0; i < size; i++) {
            buf[i] = (byte) (i & 0xFF);
        }
        return buf;
    }

    // Safe read with full validation
    public static ReadResult readFromBuffer(byte[] buffer, int index) {
        if (buffer == null) {
            return ReadResult.fail();
        }
        if (index < 0 || index >= buffer.length) {
            return ReadResult.fail();
        }
        return ReadResult.ok(buffer[index]);
    }

    // 5 test cases in main
    public static void main(String[] args) {
        byte[] buffer = allocateBuffer(10);
        int[] testIndices = new int[] {0, 5, 9, -1, 10};

        for (int idx : testIndices) {
            ReadResult res = readFromBuffer(buffer, idx);
            if (res.success) {
                System.out.println("Index " + idx + " -> " + (res.value & 0xFF));
            } else {
                System.out.println("Index " + idx + " -> invalid index");
            }
        }
    }
}