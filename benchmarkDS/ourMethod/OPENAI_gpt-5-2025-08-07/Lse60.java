import java.nio.charset.StandardCharsets;

public class Lse60 {
    private static final int MAX_SIZE = 1_048_576; // 1 MiB cap to prevent excessive memory use

    // Allocates a zero-initialized buffer of the given size and returns its contents as hex bytes.
    public static String allocateAndDumpBuffer(int size) {
        if (size < 0 || size > MAX_SIZE) {
            throw new IllegalArgumentException("Invalid size");
        }
        if (size == 0) {
            return "";
        }
        byte[] buf = new byte[size]; // Java arrays are zero-initialized
        StringBuilder sb = new StringBuilder(Math.max(0, size * 3 - 1));
        for (int i = 0; i < buf.length; i++) {
            int v = buf[i] & 0xFF;
            if (v < 16) sb.append('0');
            sb.append(Integer.toHexString(v));
            if (i + 1 < buf.length) sb.append(' ');
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        try {
            if (args.length >= 1) {
                long parsed = Long.parseLong(args[0]);
                if (parsed < 0 || parsed > MAX_SIZE) {
                    System.err.println("Error: size must be between 0 and " + MAX_SIZE);
                    System.exit(1);
                }
                String out = allocateAndDumpBuffer((int) parsed);
                if (!out.isEmpty()) {
                    System.out.println(out);
                } else {
                    System.out.println(); // print newline for empty buffer
                }
            } else {
                int[] tests = {0, 1, 8, 16, 64};
                for (int t : tests) {
                    String out = allocateAndDumpBuffer(t);
                    if (!out.isEmpty()) {
                        System.out.println(out);
                    } else {
                        System.out.println();
                    }
                }
            }
        } catch (NumberFormatException ex) {
            System.err.println("Error: invalid integer size");
            System.exit(1);
        } catch (IllegalArgumentException ex) {
            System.err.println("Error: " + ex.getMessage());
            System.exit(1);
        }
    }
}