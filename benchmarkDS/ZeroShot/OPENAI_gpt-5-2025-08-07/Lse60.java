import java.nio.charset.StandardCharsets;

public class Lse60 {
    // Secure upper bound to prevent excessive allocation/printing
    private static final int MAX_BUFFER_SIZE = 1_048_576; // 1 MiB

    // Returns a hex dump string of a buffer of the given size
    public static String secureDumpBuffer(int size) {
        if (size < 0) {
            throw new IllegalArgumentException("Size must be non-negative");
        }
        if (size > MAX_BUFFER_SIZE) {
            throw new IllegalArgumentException("Size too large (max " + MAX_BUFFER_SIZE + ")");
        }
        if (size == 0) {
            return "";
        }

        byte[] buffer = new byte[size];
        for (int i = 0; i < size; i++) {
            buffer[i] = (byte) (i & 0xFF); // deterministic initialization pattern
        }

        // Each byte contributes "XX " or "XX\n" -> exactly 3 characters per byte
        long desiredCapacity = 3L * size;
        int capacity = desiredCapacity > Integer.MAX_VALUE ? Integer.MAX_VALUE : (int) desiredCapacity;
        StringBuilder sb = new StringBuilder(capacity);
        final char[] HEX = "0123456789ABCDEF".toCharArray();

        for (int i = 0; i < size; i++) {
            int v = buffer[i] & 0xFF;
            sb.append(HEX[v >>> 4]);
            sb.append(HEX[v & 0x0F]);
            if ((i & 15) == 15) {
                sb.append('\n');
            } else {
                sb.append(' ');
            }
        }
        if ((size & 15) != 0) {
            // Replace last space with newline for the final partial line
            int lastIdx = sb.length() - 1;
            if (lastIdx >= 0 && sb.charAt(lastIdx) == ' ') {
                sb.setCharAt(lastIdx, '\n');
            }
        }

        return sb.toString();
    }

    private static void runSingle(int size) {
        try {
            String out = secureDumpBuffer(size);
            if (!out.isEmpty()) {
                System.out.print(out);
            }
        } catch (IllegalArgumentException ex) {
            System.err.println("Error: " + ex.getMessage());
            System.exit(1);
        }
    }

    public static void main(String[] args) {
        if (args.length == 1) {
            int size;
            try {
                size = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
                System.err.println("Error: Invalid number: " + args[0]);
                System.exit(1);
                return;
            }
            runSingle(size);
            return;
        }

        // 5 test cases when no argument is provided
        int[] tests = new int[] {0, 1, 16, 31, 64};
        for (int t : tests) {
            System.out.println("=== size " + t + " ===");
            runSingle(t);
        }
    }
}