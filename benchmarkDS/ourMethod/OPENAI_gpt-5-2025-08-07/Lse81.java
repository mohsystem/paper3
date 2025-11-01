import java.io.InputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse81 {

    // Allocates a buffer of exactly 1024 bytes
    public static byte[] allocateBuffer(int requestedSize) {
        final int FIXED_SIZE = 1024;
        return new byte[FIXED_SIZE];
    }

    // Reads exactly up to 10 bytes from the given InputStream into buf[0..n-1]
    // Returns number of bytes read (0..10), or -1 on error
    public static int readTenInto(InputStream in, byte[] buf) throws IOException {
        if (in == null || buf == null || buf.length < 10) {
            return -1;
        }
        int remaining = 10;
        int offset = 0;
        while (remaining > 0) {
            int r = in.read(buf, offset, remaining);
            if (r == -1) {
                break; // EOF
            }
            offset += r;
            remaining -= r;
        }
        return offset;
    }

    public static void main(String[] args) {
        try {
            for (int i = 1; i <= 5; i++) {
                System.out.println("Test " + i + ": Please type at least 10 characters and press Enter:");
                byte[] buffer = allocateBuffer(1024);
                int n = readTenInto(System.in, buffer);
                if (n < 0) {
                    System.out.println("Error: invalid buffer or input stream.");
                } else {
                    String out = new String(buffer, 0, n, StandardCharsets.UTF_8);
                    System.out.println("Read " + n + " byte(s): " + out);
                }
            }
        } catch (IOException e) {
            System.out.println("I/O error.");
        }
    }
}