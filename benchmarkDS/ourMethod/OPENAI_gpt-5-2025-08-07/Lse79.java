import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class Lse79 {

    private static final int BUF_SIZE = 1024;
    private static final int READ_100 = 100;

    public static final class ReadResult {
        public final String buf;
        public final String buf1;

        public ReadResult(String buf, String buf1) {
            this.buf = buf;
            this.buf1 = buf1;
        }
    }

    // Reads up to 100 bytes into buf and another 100 bytes into buf1 (each buffer 1024 bytes)
    public static ReadResult allocateAndCopy100Twice(InputStream in) throws IOException {
        if (in == null) {
            throw new IllegalArgumentException("InputStream cannot be null");
        }

        byte[] buf = new byte[BUF_SIZE];
        byte[] buf1 = new byte[BUF_SIZE];

        int n1 = readAtMost(in, buf, READ_100);
        int n2 = readAtMost(in, buf1, READ_100);

        String s1 = new String(buf, 0, Math.max(0, n1), StandardCharsets.UTF_8);
        String s2 = new String(buf1, 0, Math.max(0, n2), StandardCharsets.UTF_8);
        return new ReadResult(s1, s2);
    }

    // Reads up to 1024 bytes into a 1024-byte buffer and returns the string
    public static String readUpTo1024(InputStream in) throws IOException {
        if (in == null) {
            throw new IllegalArgumentException("InputStream cannot be null");
        }
        byte[] buf = new byte[BUF_SIZE];
        int n = readAtMost(in, buf, BUF_SIZE);
        return new String(buf, 0, Math.max(0, n), StandardCharsets.UTF_8);
    }

    private static int readAtMost(InputStream in, byte[] buffer, int maxToRead) throws IOException {
        if (maxToRead < 0 || maxToRead > buffer.length) {
            throw new IllegalArgumentException("Invalid maxToRead");
        }
        int total = 0;
        while (total < maxToRead) {
            int r = in.read(buffer, total, maxToRead - total);
            if (r <= 0) {
                break;
            }
            total += r;
        }
        return total;
    }

    public static void main(String[] args) throws Exception {
        // Test case 1: Less than 100 bytes
        String t1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        try (InputStream in1 = new ByteArrayInputStream(t1.getBytes(StandardCharsets.UTF_8))) {
            ReadResult res1 = allocateAndCopy100Twice(in1);
            System.out.println("T1 buf: " + res1.buf);
            System.out.println("T1 buf1: " + res1.buf1);
        }

        // Test case 2: More than 100 bytes (150 x 'x')
        String t2 = "x".repeat(150);
        try (InputStream in2 = new ByteArrayInputStream(t2.getBytes(StandardCharsets.UTF_8))) {
            ReadResult res2 = allocateAndCopy100Twice(in2);
            System.out.println("T2 buf len: " + res2.buf.length());
            System.out.println("T2 buf1 len: " + res2.buf1.length());
        }

        // Test case 3: Empty input
        String t3 = "";
        try (InputStream in3 = new ByteArrayInputStream(t3.getBytes(StandardCharsets.UTF_8))) {
            ReadResult res3 = allocateAndCopy100Twice(in3);
            System.out.println("T3 buf: '" + res3.buf + "'");
            System.out.println("T3 buf1: '" + res3.buf1 + "'");
        }

        // Test case 4: readUpTo1024 with 300 bytes
        String t4 = "y".repeat(300);
        try (InputStream in4 = new ByteArrayInputStream(t4.getBytes(StandardCharsets.UTF_8))) {
            String s4 = readUpTo1024(in4);
            System.out.println("T4 len: " + s4.length());
        }

        // Test case 5: Unicode content
        String t5 = "Hello 你好世界 🙂🚀".repeat(20); // potentially > 100 bytes
        try (InputStream in5 = new ByteArrayInputStream(t5.getBytes(StandardCharsets.UTF_8))) {
            ReadResult res5 = allocateAndCopy100Twice(in5);
            System.out.println("T5 buf (first 100 bytes as UTF-8): " + res5.buf);
            System.out.println("T5 buf1 (next 100 bytes as UTF-8): " + res5.buf1);
        }
    }
}