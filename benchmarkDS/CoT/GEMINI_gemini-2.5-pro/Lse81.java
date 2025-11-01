import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

public class Lse81 {

    /**
     * Allocates a buffer of 1024 bytes.
     *
     * @return A new byte array of size 1024.
     */
    public static byte[] allocateBuffer() {
        return new byte[1024];
    }

    /**
     * Reads exactly 10 characters from standard input and stores them in the buffer.
     * This function is designed to be safe against buffer overflows by reading a
     * fixed, small number of bytes into a much larger buffer.
     *
     * @param buf The buffer to store the read characters. Must be at least 10 bytes long.
     */
    public static void readIntoBuffer(byte[] buf) {
        if (buf == null || buf.length < 10) {
            System.err.println("Error: Buffer is null or too small.");
            return;
        }

        try {
            InputStream in = System.in;
            int bytesRead = 0;
            int totalBytesRead = 0;
            // Loop to ensure we read exactly 10 bytes, handling partial reads.
            while (totalBytesRead < 10 && (bytesRead = in.read(buf, totalBytesRead, 10 - totalBytesRead)) != -1) {
                totalBytesRead += bytesRead;
            }
            
            // Consume remaining characters in the line to prevent them from being
            // read by the next test case.
            while (in.available() > 0 && in.read() != '\n') {
                // do nothing
            }

        } catch (IOException e) {
            System.err.println("An error occurred during input reading: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Test Case " + i + " ---");
            System.out.println("Please enter exactly 10 characters and press Enter:");

            byte[] buffer = allocateBuffer();
            readIntoBuffer(buffer);

            // Create a string from the first 10 bytes for printing.
            // Using a specific charset is best practice.
            String content = new String(buffer, 0, 10, java.nio.charset.StandardCharsets.UTF_8);

            System.out.println("First 10 bytes of buffer: [" + content + "]");
            System.out.println();
        }
    }
}