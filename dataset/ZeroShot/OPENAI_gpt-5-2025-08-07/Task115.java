import java.util.*;

public class Task115 {
    public static class BitFlags {
        private final int read;     // 0-1
        private final int write;    // 0-1
        private final int execute;  // 0-1
        private final int role;     // 0-3
        private final int priority; // 0-15

        private BitFlags(int read, int write, int execute, int role, int priority) {
            this.read = read;
            this.write = write;
            this.execute = execute;
            this.role = role;
            this.priority = priority;
        }

        public static BitFlags of(int read, int write, int execute, int role, int priority) {
            validateBit(read, "read");
            validateBit(write, "write");
            validateBit(execute, "execute");
            validateRange(role, 0, 3, "role");
            validateRange(priority, 0, 15, "priority");
            return new BitFlags(read, write, execute, role, priority);
        }

        public static int buildPacked(int read, int write, int execute, int role, int priority) {
            BitFlags bf = of(read, write, execute, role, priority);
            return bf.toPacked();
        }

        public static BitFlags fromPacked(int packed) {
            int read = (packed >>> 0) & 1;
            int write = (packed >>> 1) & 1;
            int execute = (packed >>> 2) & 1;
            int role = (packed >>> 3) & 0b11;
            int priority = (packed >>> 5) & 0xF;
            return new BitFlags(read, write, execute, role, priority);
        }

        public int toPacked() {
            int p = 0;
            p |= (read & 1) << 0;
            p |= (write & 1) << 1;
            p |= (execute & 1) << 2;
            p |= (role & 0b11) << 3;
            p |= (priority & 0xF) << 5;
            return p;
        }

        private static void validateBit(int v, String name) {
            if (v != 0 && v != 1) {
                throw new IllegalArgumentException("Invalid " + name + " bit: " + v);
            }
        }

        private static void validateRange(int v, int min, int max, String name) {
            if (v < min || v > max) {
                throw new IllegalArgumentException("Invalid " + name + ": " + v);
            }
        }

        @Override
        public String toString() {
            return "BitFlags{read=" + read + ", write=" + write + ", execute=" + execute + ", role=" + role + ", priority=" + priority + "}";
        }
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 1, 0, 2, 7},
            {0, 0, 1, 1, 0},
            {1, 0, 1, 3, 15},
            {1, 1, 1, 4, 2},   // invalid role
            {1, 0, 0, 0, 16}   // invalid priority
        };
        for (int i = 0; i < tests.length; i++) {
            int[] t = tests[i];
            try {
                int packed = BitFlags.buildPacked(t[0], t[1], t[2], t[3], t[4]);
                BitFlags bf = BitFlags.fromPacked(packed);
                System.out.println("Test " + (i + 1) + ": packed=" + packed + " unpacked=" + bf.toString());
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (i + 1) + ": error=" + ex.getMessage());
            }
        }
    }
}