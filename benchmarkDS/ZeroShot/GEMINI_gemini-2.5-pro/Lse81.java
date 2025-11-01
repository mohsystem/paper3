import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

public class Lse81 {

    /**
     * Allocates a buffer of 1024 bytes.
     * @return A new byte array of size 1024.
     */
    public static byte[] allocateBuffer() {
        return new byte[1024];
    }

    /**
     * Reads exactly 10 characters (bytes) from standard input and stores them in the buffer.
     * This method is secure as it uses a bounded read, preventing a buffer overflow.
     * @param buf The buffer to store the read bytes into. Must be at least 10 bytes long.
     */
    public static void readIntoBuffer(byte[] buf) {
        if (buf == null || buf.length < 10) {
            System.err.println("Error: Buffer is null or too small.");
            return;
        }

        try {
            InputStream in = System.in;
            int bytesToRead = 10;
            int totalBytesRead = 0;
            
            // Loop to ensure all 10 bytes are read, as a single read() might not fill the buffer.
            while (totalBytesRead < bytesToRead) {
                int bytesRead = in.read(buf, totalBytesRead, bytesToRead - totalBytesRead);
                if (bytesRead == -1) {
                    // End of stream reached before 10 bytes could be read.
                    System.err.println("Warning: End of stream reached. Read " + totalBytesRead + " bytes.");
                    break;
                }
                totalBytesRead += bytesRead;
            }
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            System.out.print("Please enter at least 10 characters and press Enter: ");
            
            byte[] buffer = allocateBuffer();
            readIntoBuffer(buffer);
            
            // To display the read content, we create a string from the first 10 bytes.
            // We specify the character set for portability.
            try {
                String input = new String(buffer, 0, 10, "UTF-8");
                System.out.println("First 10 bytes read into buffer: [" + input + "]");
            } catch (java.io.UnsupportedEncodingException e) {
                System.err.println("UTF-8 encoding not supported.");
            }

            // In a real application, you might want to clear the buffer for security.
            Arrays.fill(buffer, (byte) 0);
        }
        System.out.println("\nAll test cases finished.");
    }
}