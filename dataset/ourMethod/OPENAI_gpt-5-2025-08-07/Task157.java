import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public final class Task157 {

    private static final int MAX_CAPACITY = 10_000_000;

    static final class MemoryResource implements AutoCloseable {
        private byte[] data;
        private boolean closed;

        public MemoryResource(int size) {
            if (size <= 0 || size > MAX_CAPACITY) {
                throw new IllegalArgumentException("Invalid size");
            }
            this.data = new byte[size];
            this.closed = false;
        }

        public synchronized int capacity() {
            return closed ? 0 : data.length;
        }

        public synchronized boolean isClosed() {
            return closed;
        }

        public synchronized int write(int offset, byte[] input) {
            if (closed || input == null) {
                return -1;
            }
            if (offset < 0 || offset >= data.length) {
                return -1;
            }
            int writable = Math.min(input.length, data.length - offset);
            if (writable <= 0) {
                return -1;
            }
            System.arraycopy(input, 0, data, offset, writable);
            return writable;
        }

        public synchronized byte[] read(int offset, int length) {
            if (closed) {
                return new byte[0];
            }
            if (offset < 0 || length < 0 || offset >= data.length) {
                return new byte[0];
            }
            int readable = Math.min(length, data.length - offset);
            if (readable <= 0) {
                return new byte[0];
            }
            return Arrays.copyOfRange(data, offset, offset + readable);
        }

        @Override
        public synchronized void close() {
            if (!closed) {
                Arrays.fill(data, (byte) 0);
                data = new byte[0];
                closed = true;
            }
        }
    }

    private static String asAscii(byte[] data) {
        return new String(data, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        // Test 1: Basic allocation, write, read
        try (MemoryResource mr = new MemoryResource(16)) {
            int w = mr.write(0, "Hello".getBytes(StandardCharsets.UTF_8));
            byte[] r = mr.read(0, 5);
            System.out.println("T1: written=" + w + " read=\"" + asAscii(r) + "\" cap=" + mr.capacity());
        }

        // Test 2: Partial write near end
        try (MemoryResource mr = new MemoryResource(16)) {
            int w = mr.write(13, "World!".getBytes(StandardCharsets.UTF_8)); // expect 3
            byte[] r = mr.read(0, 16);
            System.out.println("T2: written=" + w + " read=\"" + asAscii(r) + "\"");
        }

        // Test 3: Write larger than capacity, ensure clamp
        try (MemoryResource mr = new MemoryResource(8)) {
            int w = mr.write(0, "123456789".getBytes(StandardCharsets.UTF_8)); // expect 8
            byte[] r = mr.read(0, 8);
            System.out.println("T3: written=" + w + " read=\"" + asAscii(r) + "\"");
        }

        // Test 4: Use after close should fail
        MemoryResource mr4 = new MemoryResource(10);
        mr4.close();
        int w4 = mr4.write(0, "X".getBytes(StandardCharsets.UTF_8)); // expect -1
        byte[] r4 = mr4.read(0, 1); // expect empty
        System.out.println("T4: written=" + w4 + " readLen=" + r4.length + " isClosed=" + mr4.isClosed());

        // Test 5: Invalid offset (negative)
        try (MemoryResource mr = new MemoryResource(4)) {
            int w = mr.write(-1, "AB".getBytes(StandardCharsets.UTF_8)); // expect -1
            System.out.println("T5: written=" + w + " cap=" + mr.capacity());
        }
    }
}