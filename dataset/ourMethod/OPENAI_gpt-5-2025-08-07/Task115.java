import java.util.Arrays;

public final class Task115 {
    // Bit positions and masks for a 16-bit status register:
    // [bit0=isActive][bit1=isError][bits2-4=priority(3)][bits5-8=level(4)][bits9-15=code(7)]
    private static final int ACTIVE_SHIFT = 0;
    private static final int ERROR_SHIFT = 1;
    private static final int PRIORITY_SHIFT = 2;
    private static final int LEVEL_SHIFT = 5;
    private static final int CODE_SHIFT = 9;

    private static final int ACTIVE_MASK = 0x1 << ACTIVE_SHIFT;
    private static final int ERROR_MASK = 0x1 << ERROR_SHIFT;
    private static final int PRIORITY_MASK = 0x7 << PRIORITY_SHIFT;
    private static final int LEVEL_MASK = 0xF << LEVEL_SHIFT;
    private static final int CODE_MASK = 0x7F << CODE_SHIFT;

    private static final int MAX_PRIORITY = 7;
    private static final int MAX_LEVEL = 15;
    private static final int MAX_CODE = 127;

    public static final class Status {
        public final boolean isActive;
        public final boolean isError;
        public final int priority;
        public final int level;
        public final int code;

        public Status(boolean isActive, boolean isError, int priority, int level, int code) {
            validateField("priority", priority, 0, MAX_PRIORITY);
            validateField("level", level, 0, MAX_LEVEL);
            validateField("code", code, 0, MAX_CODE);
            this.isActive = isActive;
            this.isError = isError;
            this.priority = priority;
            this.level = level;
            this.code = code;
        }

        @Override
        public String toString() {
            return "Status{isActive=" + isActive +
                   ", isError=" + isError +
                   ", priority=" + priority +
                   ", level=" + level +
                   ", code=" + code + "}";
        }
    }

    private static void validateField(String name, int value, int min, int max) {
        if (value < min || value > max) {
            throw new IllegalArgumentException("Invalid " + name + " value: " + value + " (allowed " + min + "-" + max + ")");
        }
    }

    public static int pack(boolean isActive, boolean isError, int priority, int level, int code) {
        validateField("priority", priority, 0, MAX_PRIORITY);
        validateField("level", level, 0, MAX_LEVEL);
        validateField("code", code, 0, MAX_CODE);
        int v = 0;
        v |= (isActive ? 1 : 0) << ACTIVE_SHIFT;
        v |= (isError ? 1 : 0) << ERROR_SHIFT;
        v |= (priority & 0x7) << PRIORITY_SHIFT;
        v |= (level & 0xF) << LEVEL_SHIFT;
        v |= (code & 0x7F) << CODE_SHIFT;
        return v & 0xFFFF;
    }

    public static Status unpack(int packed) {
        if ((packed & ~0xFFFF) != 0) {
            throw new IllegalArgumentException("Packed value must be 16-bit unsigned");
        }
        boolean isActive = ((packed & ACTIVE_MASK) >>> ACTIVE_SHIFT) == 1;
        boolean isError = ((packed & ERROR_MASK) >>> ERROR_SHIFT) == 1;
        int priority = (packed & PRIORITY_MASK) >>> PRIORITY_SHIFT;
        int level = (packed & LEVEL_MASK) >>> LEVEL_SHIFT;
        int code = (packed & CODE_MASK) >>> CODE_SHIFT;
        return new Status(isActive, isError, priority, level, code);
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            // Each test: {isActive(0/1), isError(0/1), priority, level, code}
            {1, 0, 3, 8, 42},
            {0, 1, 7, 15, 127},
            {1, 1, 0, 0, 0},
            {0, 0, 5, 9, 100},
            {1, 0, 9, 2, 5} // invalid priority (9 > 7)
        };

        for (int i = 0; i < tests.length; i++) {
            int[] t = tests[i];
            System.out.println("Test " + (i + 1) + " input: " + Arrays.toString(t));
            try {
                int packed = pack(t[0] != 0, t[1] != 0, t[2], t[3], t[4]);
                System.out.println("Packed: " + packed);
                Status s = unpack(packed);
                System.out.println("Unpacked: " + s.toString());
            } catch (IllegalArgumentException ex) {
                System.out.println("Error: " + ex.getMessage());
            }
        }
    }
}