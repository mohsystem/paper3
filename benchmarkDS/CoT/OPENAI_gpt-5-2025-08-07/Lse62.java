// Chain-of-Through process:
// 1) Problem understanding: Read a file into a buffer, compute 32-bit checksum (sum of all bytes modulo 2^32).
//    If checksum == 0, buffer is considered empty. Save bytesRead and checksum to an output file.
// 2) Security requirements: Use buffered streaming to avoid memory issues; validate I/O operations; close resources safely.
// 3) Secure coding generation: Use try-with-resources; avoid loading entire file; handle exceptions; avoid integer overflows via masking.
// 4) Code review: Ensure proper error handling, resource cleanup, and correct numeric handling of unsigned bytes.
// 5) Secure code output: Final code incorporates safe I/O, clear separation of concerns, and deterministic checksum.
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;

public class Lse62 {
    public static final class Result {
        public final long bytesRead;
        public final long checksum; // unsigned 32-bit stored in long

        public Result(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }

        @Override
        public String toString() {
            return "Result{bytesRead=" + bytesRead + ", checksum=" + (checksum & 0xFFFFFFFFL) + "}";
        }
    }

    // Reads inputPath, computes checksum, writes "bytesRead" and "checksum" to outputPath, returns Result.
    public static Result processFile(String inputPath, String outputPath) throws IOException {
        final int BUF_SIZE = 8192;
        long bytesReadTotal = 0L;
        long checksum = 0L; // We'll keep it masked to 32 bits

        try (BufferedInputStream in = new BufferedInputStream(new FileInputStream(inputPath), BUF_SIZE)) {
            byte[] buf = new byte[BUF_SIZE];
            int n;
            while ((n = in.read(buf)) != -1) {
                bytesReadTotal += n;
                for (int i = 0; i < n; i++) {
                    checksum = (checksum + (buf[i] & 0xFF)) & 0xFFFFFFFFL;
                }
            }
        }

        // By specification: if checksum == 0 -> buffer considered empty
        try (OutputStream out = new BufferedOutputStream(new FileOutputStream(outputPath, false))) {
            String content = "bytesRead=" + bytesReadTotal + "\nchecksum=" + (checksum & 0xFFFFFFFFL) + "\n";
            out.write(content.getBytes(StandardCharsets.UTF_8));
            out.flush();
        }

        return new Result(bytesReadTotal, checksum & 0xFFFFFFFFL);
    }

    private static void writeBytes(String path, byte[] data) throws IOException {
        try (OutputStream out = new BufferedOutputStream(new FileOutputStream(path, false))) {
            out.write(data);
            out.flush();
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] inFiles = {
            "java_in_1_empty.bin",
            "java_in_2_hello.txt",
            "java_in_3_zeros.bin",
            "java_in_4_seq.bin",
            "java_in_5_large.bin"
        };
        String[] outFiles = {
            "java_out_1.txt",
            "java_out_2.txt",
            "java_out_3.txt",
            "java_out_4.txt",
            "java_out_5.txt"
        };

        try {
            // Test 1: empty file
            writeBytes(inFiles[0], new byte[0]);

            // Test 2: "Hello, world!"
            writeBytes(inFiles[1], "Hello, world!".getBytes(StandardCharsets.UTF_8));

            // Test 3: 1024 zeros
            writeBytes(inFiles[2], new byte[1024]);

            // Test 4: 4096 bytes of 0..255 pattern
            byte[] seq = new byte[4096];
            for (int i = 0; i < seq.length; i++) seq[i] = (byte)(i & 0xFF);
            writeBytes(inFiles[3], seq);

            // Test 5: 10000 bytes repeating ASCII A..Z
            byte[] large = new byte[10000];
            for (int i = 0; i < large.length; i++) large[i] = (byte)('A' + (i % 26));
            writeBytes(inFiles[4], large);

            for (int i = 0; i < inFiles.length; i++) {
                Result r = processFile(inFiles[i], outFiles[i]);
                System.out.println("Test " + (i + 1) + " -> " + r);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}