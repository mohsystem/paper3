import java.io.*;
import java.nio.file.*;
import java.util.Arrays;
import java.util.regex.Pattern;

/*
Chain-of-Through process in code generation:
1) Problem understanding:
   - Implement an img(String fileParam) that safely reads a file (binary) from a predefined directory and returns its bytes.
   - This function represents the handler invoked for an /img route where 'file' query parameter provides the filename.
2) Security requirements:
   - Prevent path traversal, disallow separators, restrict filename to safe characters, limit filename length, restrict to a base directory.
3) Secure coding generation:
   - Validate input with a strict regex.
   - Resolve canonical paths and ensure they remain within BASE_DIR.
   - Handle I/O safely; on failures return empty byte array.
4) Code review:
   - Line-by-line checks ensure no uncontrolled file access, no unsafe concatenations, handled exceptions.
5) Secure code output:
   - Final code embodies mitigations and includes tests.
*/

public class Lse48 {

    private static final Path BASE_DIR = Paths.get("static").toAbsolutePath().normalize();
    private static final Pattern SAFE_NAME = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");

    // img() simulates the route handler for /img?file=<name>
    // It returns the file content as byte[] if safe and exists; otherwise empty byte[].
    public static byte[] img(String fileParam) {
        try {
            // Validate input
            if (fileParam == null) return new byte[0];
            if (!SAFE_NAME.matcher(fileParam).matches()) return new byte[0];
            // Prevent hidden special names that could be risky (optional hardening)
            if (fileParam.equals(".") || fileParam.equals("..")) return new byte[0];

            // Resolve within base directory
            Path target = BASE_DIR.resolve(fileParam).normalize();
            if (!target.startsWith(BASE_DIR)) return new byte[0]; // Path traversal defense

            // Read all bytes
            if (!Files.exists(target) || !Files.isRegularFile(target)) return new byte[0];
            return Files.readAllBytes(target);
        } catch (Exception e) {
            return new byte[0];
        }
    }

    private static void ensureBaseAndFixtures() throws IOException {
        if (!Files.exists(BASE_DIR)) {
            Files.createDirectories(BASE_DIR);
        }
        // Create fixture files
        Path f1 = BASE_DIR.resolve("index.txt");
        if (!Files.exists(f1)) {
            Files.write(f1, "Hello from index\n".getBytes());
        }
        Path f2 = BASE_DIR.resolve("image.bin");
        if (!Files.exists(f2)) {
            byte[] data = new byte[16];
            for (int i = 0; i < data.length; i++) data[i] = (byte) (i * 7);
            Files.write(f2, data);
        }
    }

    private static void printResult(String label, byte[] data) {
        System.out.println(label + " -> size=" + (data == null ? -1 : data.length) +
                ", head=" + Arrays.toString(Arrays.copyOfRange(data == null ? new byte[0] : data, 0, Math.min(8, data.length))));
    }

    public static void main(String[] args) throws Exception {
        ensureBaseAndFixtures();

        // 5 test cases
        byte[] r1 = img("index.txt");            // valid
        byte[] r2 = img("image.bin");            // valid
        byte[] r3 = img("../etc/passwd");        // traversal attempt -> empty
        byte[] r4 = img("bad$name.txt");         // invalid chars -> empty
        byte[] r5 = img("missing.txt");          // non-existent -> empty

        printResult("TC1 index.txt", r1);
        printResult("TC2 image.bin", r2);
        printResult("TC3 ../etc/passwd", r3);
        printResult("TC4 bad$name.txt", r4);
        printResult("TC5 missing.txt", r5);
    }
}