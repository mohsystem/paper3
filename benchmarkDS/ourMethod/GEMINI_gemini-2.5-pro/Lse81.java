import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class Lse81 {

    private static final int BUFFER_SIZE = 1024;
    private static final int READ_SIZE = 10;

    /**
     * Allocates a buffer of BUFFER_SIZE bytes.
     * @return A new byte array of size 1024.
     */
    public static byte[] allocateBuffer() {
        return new byte[BUFFER_SIZE];
    }

    /**
     * Reads up to READ_SIZE characters from the provided InputStream and stores them in the buffer.
     * @param buf The buffer to store the characters in. It must be at least READ_SIZE bytes long.
     * @param in The InputStream to read from (e.g., System.in).
     * @return The number of bytes actually read, or -1 if the end of the stream is reached.
     * @throws IOException if an I/O error occurs.
     */
    public static int readIntoBuffer(byte[] buf, InputStream in) throws IOException {
        if (buf == null || buf.length < READ_SIZE) {
            System.err.println("Buffer is null or too small.");
            return 0;
        }

        System.out.printf("Please enter characters. The first %d will be read: ", READ_SIZE);
        
        // Read is a blocking call. It will read up to READ_SIZE bytes.
        // It's a safe, bounded read and will not overflow the buffer given the check above.
        return in.read(buf, 0, READ_SIZE);
    }

    /**
     * Main method with 5 test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        System.out.println("Running 5 test cases.");
        System.out.println("NOTE: On some systems, extra characters on a line may be used as input for the next test case.");

        for (int i = 1; i <= 5; i++) {
            System.out.printf("\n--- Test Case %d ---\n", i);
            byte[] buffer = allocateBuffer();
            try {
                int bytesRead = readIntoBuffer(buffer, System.in);

                if (bytesRead > 0) {
                    // Convert the read part of the buffer to a string for printing.
                    // Specify charset for correctness.
                    String content = new String(buffer, 0, bytesRead, StandardCharsets.UTF_8);
                    System.out.printf("Read %d bytes: \"%s\"\n", bytesRead, content);
                } else if (bytesRead == -1) {
                    System.out.println("End of stream reached.");
                    break; // Exit loop on EOF
                } else {
                    System.out.println("No bytes were read.");
                }

                // In an interactive loop, it's good practice to consume the rest of the line.
                // The following line is a simple way to do this for interactive console testing.
                if (bytesRead != -1 && System.in.available() > 0) {
                    // This is not guaranteed to consume the entire line but works for many simple cases.
                    long skipped = System.in.skip(System.in.available());
                    if (skipped == 0) { // If nothing was skipped, read one byte to unblock (e.g., the newline)
                        System.in.read();
                    }
                }

            } catch (IOException e) {
                System.err.println("An I/O error occurred: " + e.getMessage());
                break; // Exit loop on error
            }
        }
    }
}