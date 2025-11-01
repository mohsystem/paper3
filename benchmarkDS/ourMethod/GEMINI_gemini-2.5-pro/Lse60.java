public class Lse60 {

    private static final int MAX_BUFFER_SIZE = 65536; // 64 KB limit

    /**
     * Allocates a buffer of the specified size, prints its contents, and then the memory is garbage collected.
     * @param sizeStr The desired size of the buffer as a string.
     */
    public static void allocateAndPrintBuffer(String sizeStr) {
        if (sizeStr == null) {
            System.err.println("Error: Size argument cannot be null.");
            return;
        }

        long sizeLong;
        try {
            // Use long to catch inputs that would overflow an int before validation
            sizeLong = Long.parseLong(sizeStr);
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid number format for size.");
            return;
        }

        if (sizeLong < 0) {
            System.err.println("Error: Buffer size cannot be negative.");
            return;
        }

        if (sizeLong > MAX_BUFFER_SIZE) {
            System.err.println("Error: Buffer size exceeds the maximum allowed limit of " + MAX_BUFFER_SIZE + " bytes.");
            return;
        }
        
        int size = (int) sizeLong;

        try {
            // In Java, a new byte array is automatically initialized to zeros.
            byte[] buffer = new byte[size];
            
            System.out.println("Allocated a buffer of size: " + size);
            System.out.println("Buffer contents (hex):");
            if (size == 0) {
                System.out.println("(empty)");
            } else {
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < buffer.length; i++) {
                    sb.append(String.format("%02x ", buffer[i]));
                    if ((i + 1) % 16 == 0 && i + 1 < buffer.length) {
                        sb.append("\n");
                    }
                }
                System.out.println(sb.toString().trim());
            }

        } catch (OutOfMemoryError e) {
            System.err.println("Error: Failed to allocate buffer due to insufficient memory.");
        } finally {
             System.out.println("--------------------");
        }
    }

    /**
     * Main method. If a command-line argument is provided, it's used as the buffer size.
     * Otherwise, a set of test cases is run.
     * @param args Command line arguments. Expects one argument: the buffer size.
     */
    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java Lse60 <buffer_size>");
            System.out.println("\nRunning test cases:");
            
            System.out.println("Test Case 1: Valid size (16)");
            allocateAndPrintBuffer("16");

            System.out.println("Test Case 2: Zero size");
            allocateAndPrintBuffer("0");

            System.out.println("Test Case 3: Negative size (-5, invalid)");
            allocateAndPrintBuffer("-5");

            System.out.println("Test Case 4: Non-numeric size ('abc', invalid)");
            allocateAndPrintBuffer("abc");

            System.out.println("Test Case 5: Size too large (" + (MAX_BUFFER_SIZE + 1) + ", invalid)");
            allocateAndPrintBuffer(String.valueOf(MAX_BUFFER_SIZE + 1));
            return;
        }

        allocateAndPrintBuffer(args[0]);
    }
}