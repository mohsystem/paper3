import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Locale;
import java.util.Set;

public class Lse66 {
    // Chain-of-Through process:
    // 1) Problem understanding:
    //    - Functionality: Accept a PDF file (as bytes), validate it, and save it securely in "./uploads".
    //    - Input: byte[] data (file content), String originalFilename (for logging/metadata only).
    //    - Output: String path to the saved file on success; throws on failure.
    //
    // 2) Security requirements:
    //    - Validate content type by magic header "%PDF-".
    //    - Enforce a maximum file size to prevent DoS via oversized uploads.
    //    - Do not trust client-supplied filename; generate a safe, random name.
    //    - Create uploads directory with safe permissions; avoid path traversal.
    //    - Use atomic/exclusive file creation to prevent overwrites/race conditions when possible.
    //
    // 3) Secure coding generation:
    //    - Implement checks and safe file handling as described.
    //
    // 4) Code review:
    //    - Line-by-line inspection embedded via comments.
    //
    // 5) Secure code output:
    //    - Final implementation below.

    private static final int MAX_SIZE = 1 * 1024 * 1024; // 1 MiB limit for demo/tests.
    private static final SecureRandom RNG = new SecureRandom();

    private static void ensureUploadsDir() throws IOException {
        Path uploads = Paths.get("uploads").toAbsolutePath().normalize();
        if (Files.notExists(uploads)) {
            Files.createDirectories(uploads);
            try {
                // On POSIX systems, restrict directory permissions to 700
                Files.setPosixFilePermissions(uploads, Set.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                ));
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX filesystem (e.g., Windows) - ignore
            }
        }
    }

    private static String randomHex(int bytes) {
        byte[] b = new byte[bytes];
        RNG.nextBytes(b);
        StringBuilder sb = new StringBuilder(bytes * 2);
        for (byte value : b) {
            sb.append(String.format(Locale.ROOT, "%02x", value));
        }
        return sb.toString();
    }

    private static boolean hasPdfHeader(byte[] data) {
        if (data == null || data.length < 5) return false;
        return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46 && data[4] == 0x2D; // "%PDF-"
    }

    public static String savePdf(byte[] data, String originalFilename) throws IOException {
        if (data == null) throw new IllegalArgumentException("data is null");
        if (data.length < 5) throw new IllegalArgumentException("data too small to be a PDF");
        if (data.length > MAX_SIZE) throw new IllegalArgumentException("file too large");
        if (!hasPdfHeader(data)) throw new IllegalArgumentException("invalid PDF header");

        ensureUploadsDir();

        Path uploads = Paths.get("uploads").toAbsolutePath().normalize();
        // Ignore original filename for security; generate random safe name
        String fileName = "pdf-" + randomHex(12) + ".pdf";
        Path target = uploads.resolve(fileName).normalize();

        if (!target.startsWith(uploads)) {
            throw new SecurityException("Invalid path resolution");
        }

        // Write file with CREATE_NEW to avoid overwriting existing files
        Files.write(target, data, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);

        return target.toString();
    }

    public static void main(String[] args) {
        // 5 Test cases
        // 1) Valid small PDF
        try {
            byte[] pdf1 = ("%PDF-1.4\n1 0 obj\n<<>>\nendobj\nxref\n0 1\n0000000000 65535 f \ntrailer\n<<>>\nstartxref\n9\n%%EOF")
                    .getBytes(StandardCharsets.US_ASCII);
            String path1 = savePdf(pdf1, "test1.pdf");
            System.out.println("Test1 OK: " + path1);
        } catch (Exception e) {
            System.out.println("Test1 FAILED: " + e.getMessage());
        }

        // 2) Invalid header
        try {
            byte[] notPdf = "HELLO".getBytes(StandardCharsets.US_ASCII);
            String path2 = savePdf(notPdf, "malicious.pdf");
            System.out.println("Test2 FAILED: " + path2);
        } catch (Exception e) {
            System.out.println("Test2 OK (rejected): " + e.getMessage());
        }

        // 3) Oversized file
        try {
            byte[] big = new byte[MAX_SIZE + 10];
            // Put PDF header to avoid header error
            byte[] header = "%PDF-".getBytes(StandardCharsets.US_ASCII);
            System.arraycopy(header, 0, big, 0, header.length);
            String path3 = savePdf(big, "big.pdf");
            System.out.println("Test3 FAILED: " + path3);
        } catch (Exception e) {
            System.out.println("Test3 OK (rejected): " + e.getMessage());
        }

        // 4) Path traversal attempt in filename (ignored)
        try {
            byte[] pdf4 = ("%PDF-1.7\n%%EOF").getBytes(StandardCharsets.US_ASCII);
            String path4 = savePdf(pdf4, "../evil.pdf");
            System.out.println("Test4 OK: " + path4);
        } catch (Exception e) {
            System.out.println("Test4 FAILED: " + e.getMessage());
        }

        // 5) Null/empty data
        try {
            String path5 = savePdf(null, "null.pdf");
            System.out.println("Test5 FAILED: " + path5);
        } catch (Exception e) {
            System.out.println("Test5 OK (rejected): " + e.getMessage());
        }
    }
}