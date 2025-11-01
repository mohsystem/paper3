import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

public class Lse79 {
    // Chain-of-Through process:
    // 1) Problem understanding:
    //    - Allocate two 1024-byte buffers (buf, buf1). Copy up to 100 characters from an input stream into each, sequentially.
    //    - Allocate one 1024-byte buffer and read up to 1024 characters from an input stream.
    // 2) Security requirements:
    //    - Avoid buffer overflows by bounds-checking copies.
    //    - Validate inputs and handle I/O errors.
    // 3) Secure coding generation:
    //    - Use safe loops with strict upper bounds and check EOF.
    // 4) Code review:
    //    - Each loop is bounded; arrays sized 1024; return actual lengths.
    // 5) Secure code output:
    //    - Final code here follows the above.

    public static class Copy100Result {
        public final byte[] bufData;  // first up to 100 bytes read
        public final int bufLen;
        public final byte[] buf1Data; // next up to 100 bytes read
        public final int buf1Len;
        Copy100Result(byte[] b, int bl, byte[] b1, int b1l) {
            this.bufData = b; this.bufLen = bl; this.buf1Data = b1; this.buf1Len = b1l;
        }
    }

    public static class Read1024Result {
        public final byte[] data;
        public final int length;
        Read1024Result(byte[] d, int l) { this.data = d; this.length = l; }
    }

    // Allocates two 1024-byte buffers (buf, buf1) and reads up to 100 bytes into each from 'in' sequentially.
    public static Copy100Result allocateAndCopy100Twice(InputStream in) throws IOException {
        if (in == null) throw new IllegalArgumentException("InputStream cannot be null");

        byte[] buf = new byte[1024];
        byte[] buf1 = new byte[1024];

        int read1 = 0;
        while (read1 < 100) {
            int b = in.read();
            if (b == -1) break;
            buf[read1++] = (byte) b;
        }

        int read2 = 0;
        while (read2 < 100) {
            int b = in.read();
            if (b == -1) break;
            buf1[read2++] = (byte) b;
        }

        byte[] out1 = new byte[read1];
        System.arraycopy(buf, 0, out1, 0, read1);
        byte[] out2 = new byte[read2];
        System.arraycopy(buf1, 0, out2, 0, read2);

        // buffers buf and buf1 are local and will be GC'd; addresses were stored in variables named buf, buf1 as required.
        return new Copy100Result(out1, read1, out2, read2);
    }

    // Allocates a 1024-byte buffer and reads up to 1024 bytes from 'in'.
    public static Read1024Result allocateAndReadUpTo1024(InputStream in) throws IOException {
        if (in == null) throw new IllegalArgumentException("InputStream cannot be null");
        byte[] buf = new byte[1024];
        int total = 0;
        while (total < 1024) {
            int r = in.read(buf, total, 1024 - total);
            if (r <= 0) break;
            total += r;
        }
        byte[] out = new byte[total];
        System.arraycopy(buf, 0, out, 0, total);
        return new Read1024Result(out, total);
    }

    private static String bytesToPrintable(byte[] b, int len) {
        String s = new String(b, 0, len, StandardCharsets.UTF_8);
        return s.replace("\n", "\\n").replace("\r", "\\r");
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        String[] inputs = new String[] {
            "HelloWorld", // <100
            "A".repeat(100), // exactly 100
            "B".repeat(150), // between 100 and 200
            "C".repeat(220), // >200
            "D".repeat(1500) // >1024
        };

        for (int i = 0; i < inputs.length; i++) {
            System.out.println("=== Java Test Case " + (i+1) + " ===");
            byte[] src = inputs[i].getBytes(StandardCharsets.UTF_8);

            Copy100Result r100 = allocateAndCopy100Twice(new ByteArrayInputStream(src));
            System.out.println("First 100 buf len=" + r100.bufLen + " data=" + bytesToPrintable(r100.bufData, r100.bufLen));
            System.out.println("Second 100 buf1 len=" + r100.buf1Len + " data=" + bytesToPrintable(r100.buf1Data, r100.buf1Len));

            Read1024Result r1024 = allocateAndReadUpTo1024(new ByteArrayInputStream(src));
            System.out.println("Read up to 1024 len=" + r1024.length);
        }
    }
}