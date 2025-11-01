import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse81 {

    // Allocates a buffer of 1024 bytes
    public static byte[] allocateBuffer() {
        return new byte[1024];
    }

    // Reads up to 10 bytes from the provided InputStream (e.g., System.in) into buf
    // Returns the number of bytes read
    public static int readTenCharsToBuffer(InputStream in, byte[] buf) throws IOException {
        if (in == null) {
            throw new IllegalArgumentException("InputStream cannot be null");
        }
        if (buf == null) {
            throw new IllegalArgumentException("Buffer cannot be null");
        }
        if (buf.length < 10) {
            throw new IllegalArgumentException("Buffer must be at least 10 bytes long");
        }

        int totalRead = 0;
        while (totalRead < 10) {
            int b = in.read();
            if (b == -1) {
                break; // EOF
            }
            buf[totalRead] = (byte) b;
            totalRead++;
        }
        return totalRead;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "HelloWorld!",      // >10
            "Short",            // <10
            "1234567890ABC",    // >10
            "",                 // empty
            "ExactTen!!"        // exactly 10
        };

        for (int i = 0; i < tests.length; i++) {
            byte[] buffer = allocateBuffer();
            int readCount = 0;
            try (ByteArrayInputStream bais = new ByteArrayInputStream(tests[i].getBytes(StandardCharsets.UTF_8))) {
                readCount = readTenCharsToBuffer(bais, buffer);
            } catch (IOException e) {
                System.out.println("Test " + (i + 1) + " encountered an IO error: " + e.getMessage());
                continue;
            } catch (IllegalArgumentException e) {
                System.out.println("Test " + (i + 1) + " invalid argument: " + e.getMessage());
                continue;
            }

            String readStr = new String(Arrays.copyOfRange(buffer, 0, readCount), StandardCharsets.UTF_8);
            System.out.println("Test " + (i + 1) + ": readCount=" + readCount + ", data=\"" + readStr + "\"");
        }
    }
}