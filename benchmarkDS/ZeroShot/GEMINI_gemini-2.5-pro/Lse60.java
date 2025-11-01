public class Lse60 {
    // A reasonable upper limit for buffer size to prevent Denial of Service attacks
    private static final int MAX_BUFFER_SIZE = 1024 * 1024; // 1MB

    /**
     * Allocates a buffer of a specified size and prints its contents.
     * @param size The desired size of the buffer.
     */
    public static void processBuffer(int size) {
        System.out.println("--- Processing request for buffer of size: " + size + " ---");
        if (size < 0) {
            System.err.println("Error: Buffer size cannot be negative.");
            System.out.println("--- Finished processing ---");
            System.out.println();
            return;
        }
        if (size > MAX_BUFFER_SIZE) {
            System.err.println("Error: Requested buffer size " + size + " exceeds maximum allowed size of " + MAX_BUFFER_SIZE + ".");
            System.out.println("--- Finished processing ---");
            System.out.println();
            return;
        }

        try {
            // In Java, byte arrays are automatically initialized to zero.
            byte[] buffer = new byte[size];
            System.out.println("Buffer of size " + size + " allocated successfully.");
            
            // The prompt asks to print the contents. In Java, this will be all zeros.
            if (size > 0) {
                 System.out.print("Buffer contents: ");
                 int printLimit = Math.min(size, 32); // Print up to 32 bytes for brevity
                 for (int i = 0; i < printLimit; i++) {
                     System.out.print(String.format("%02X ", buffer[i]));
                 }
                 if (size > printLimit) {
                     System.out.print("...");
                 }
                 System.out.println();
            } else {
                System.out.println("Buffer is empty.");
            }
        } catch (OutOfMemoryError e) {
            System.err.println("Error: Failed to allocate buffer of size " + size + ". Out of memory.");
        }
        System.out.println("--- Finished processing ---");
        System.out.println();
    }

    /**
     * Main entry point. Processes command-line arguments or runs test cases.
     * @param args Command-line arguments. Expects a single integer for buffer size.
     */
    public static void main(String[] args) {
        if (args.length == 1) {
            try {
                int size = Integer.parseInt(args[0]);
                processBuffer(size);
            } catch (NumberFormatException e) {
                System.err.println("Error: Invalid number format. Please provide an integer for the buffer size.");
            }
        } else if (args.length == 0) {
            System.out.println("No command-line argument provided. Running test cases.");
            // 5 test cases
            processBuffer(16);          // Test Case 1: A small, valid size
            processBuffer(0);           // Test Case 2: A zero size
            processBuffer(1);           // Test Case 3: A size of 1
            processBuffer(-10);         // Test Case 4: An invalid negative size
            processBuffer(2000000);     // Test Case 5: A size that exceeds our custom limit
        } else {
            System.err.println("Usage: java Lse60 <buffer_size>");
        }
    }
}