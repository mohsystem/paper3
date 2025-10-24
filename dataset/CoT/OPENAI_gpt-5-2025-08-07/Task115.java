// Chain-of-Through secure code generation process embedded as comments.
// 1) Problem understanding: We define a compact permissions representation using bit fields.
//    Java has no native bit fields, so we pack fields into a 16-bit short via masking/shifting.
//    Fields: read(1), write(1), execute(1), role(2), level(5) => total 10 bits.
// 2) Security requirements: Validate inputs are within range, avoid sign issues with short,
//    prevent invalid shifts, and encapsulate state.
// 3) Secure coding generation: Provide factory methods with validation, immutable storage,
//    safe masking, and conversion utilities.
// 4) Code review: Each bit manipulation is masked and range-checked. No external input parsing.
// 5) Secure code output: Final code ensures correctness and safety.
public class Task115 {

    public static final class BitPermissions {
        // Internal packed representation (lower 10 bits used)
        private final int packed; // store as int to avoid sign issues; guarantee 0..65535

        // Bit widths and positions
        private static final int READ_SHIFT = 0;
        private static final int WRITE_SHIFT = 1;
        private static final int EXEC_SHIFT = 2;
        private static final int ROLE_SHIFT = 3;   // 2 bits
        private static final int LEVEL_SHIFT = 5;  // 5 bits

        private static final int READ_MASK = 0x1;
        private static final int WRITE_MASK = 0x1;
        private static final int EXEC_MASK = 0x1;
        private static final int ROLE_MASK = 0x3;
        private static final int LEVEL_MASK = 0x1F;

        private BitPermissions(int packed) {
            this.packed = packed & 0xFFFF;
        }

        public static BitPermissions fromFields(int read, int write, int execute, int role, int level) {
            // Validate ranges to prevent misuse
            if ((read & ~READ_MASK) != 0) throw new IllegalArgumentException("read out of range");
            if ((write & ~WRITE_MASK) != 0) throw new IllegalArgumentException("write out of range");
            if ((execute & ~EXEC_MASK) != 0) throw new IllegalArgumentException("execute out of range");
            if ((role & ~ROLE_MASK) != 0) throw new IllegalArgumentException("role out of range");
            if ((level & ~LEVEL_MASK) != 0) throw new IllegalArgumentException("level out of range");

            int p = 0;
            p |= (read   & READ_MASK)  << READ_SHIFT;
            p |= (write  & WRITE_MASK) << WRITE_SHIFT;
            p |= (execute& EXEC_MASK)  << EXEC_SHIFT;
            p |= (role   & ROLE_MASK)  << ROLE_SHIFT;
            p |= (level  & LEVEL_MASK) << LEVEL_SHIFT;
            return new BitPermissions(p);
        }

        public static BitPermissions fromShort(short packed) {
            return new BitPermissions(packed & 0xFFFF);
        }

        public int getRead()   { return (packed >>> READ_SHIFT) & READ_MASK; }
        public int getWrite()  { return (packed >>> WRITE_SHIFT) & WRITE_MASK; }
        public int getExecute(){ return (packed >>> EXEC_SHIFT) & EXEC_MASK; }
        public int getRole()   { return (packed >>> ROLE_SHIFT) & ROLE_MASK; }
        public int getLevel()  { return (packed >>> LEVEL_SHIFT) & LEVEL_MASK; }

        public short toShort() {
            return (short)(packed & 0xFFFF);
        }

        @Override
        public String toString() {
            return "BitPermissions(read=" + getRead() +
                   ", write=" + getWrite() +
                   ", execute=" + getExecute() +
                   ", role=" + getRole() +
                   ", level=" + getLevel() +
                   ", packed=0x" + String.format("%04X", packed & 0xFFFF) + ")";
        }
    }

    // Utility method that accepts inputs and returns packed short
    public static short packPermissions(int read, int write, int execute, int role, int level) {
        return BitPermissions.fromFields(read, write, execute, role, level).toShort();
    }

    // Utility that accepts a packed short and returns a BitPermissions object
    public static BitPermissions unpackPermissions(short packed) {
        return BitPermissions.fromShort(packed);
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1,0,1,2,17},
            {0,1,1,1,31},
            {1,1,1,3,0},
            {0,0,0,0,5},
            {1,1,0,2,29}
        };

        for (int i = 0; i < tests.length; i++) {
            int[] t = tests[i];
            BitPermissions bp = BitPermissions.fromFields(t[0], t[1], t[2], t[3], t[4]);
            System.out.println("Test " + (i+1) + " -> " + bp.toString());
            short packed = packPermissions(t[0], t[1], t[2], t[3], t[4]);
            BitPermissions unpacked = unpackPermissions(packed);
            System.out.println("Unpacked " + (i+1) + " -> " + unpacked.toString());
        }
    }
}