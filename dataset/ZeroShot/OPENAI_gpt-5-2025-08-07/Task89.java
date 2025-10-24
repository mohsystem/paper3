import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;

public class Task89 {

    private static final int MAX_PASSWORD_LENGTH = 1024;

    public static boolean comparePasswordFromFile(String filePath, char[] userPassword) {
        if (filePath == null || userPassword == null) {
            return false;
        }

        char[] stored = null;
        try (BufferedReader br = Files.newBufferedReader(Paths.get(filePath), StandardCharsets.UTF_8)) {
            stored = readFirstLineLimited(br, MAX_PASSWORD_LENGTH);
            if (stored == null) {
                stored = new char[0]; // empty file treated as empty password
            }
            return constantTimeEquals(stored, userPassword);
        } catch (IOException e) {
            return false;
        } finally {
            // Attempt to clear sensitive data
            if (stored != null) {
                zeroCharArray(stored);
            }
            zeroCharArray(userPassword);
        }
    }

    private static char[] readFirstLineLimited(Reader reader, int maxLen) throws IOException {
        StringBuilder sb = new StringBuilder(Math.min(maxLen, 64));
        int ch;
        int count = 0;
        while ((ch = reader.read()) != -1) {
            if (ch == '\n') break;
            if (ch == '\r') {
                // consume a following '\n' if present without adding to buffer
                reader.mark(1);
                int next = reader.read();
                if (next != '\n') {
                    reader.reset();
                }
                break;
            }
            if (count >= maxLen) {
                // Exceeded allowed length
                return null;
            }
            sb.append((char) ch);
            count++;
        }
        return sb.toString().toCharArray();
    }

    private static boolean constantTimeEquals(char[] a, char[] b) {
        int lenA = a.length;
        int lenB = b.length;
        int maxLen = Math.max(lenA, lenB);
        int diff = lenA ^ lenB;
        for (int i = 0; i < maxLen; i++) {
            char ca = i < lenA ? a[i] : 0;
            char cb = i < lenB ? b[i] : 0;
            diff |= (ca ^ cb);
        }
        return diff == 0;
    }

    private static void zeroCharArray(char[] arr) {
        if (arr != null) {
            for (int i = 0; i < arr.length; i++) {
                arr[i] = 0;
            }
        }
    }

    private static Path createSecureTempFile(String content) throws IOException {
        Path tempFile;
        try {
            // Try POSIX-secure perms if supported
            EnumSet<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            FileAttribute<?> attr = PosixFilePermissions.asFileAttribute(perms);
            tempFile = Files.createTempFile("task89_", ".pwd", attr);
        } catch (UnsupportedOperationException ex) {
            tempFile = Files.createTempFile("task89_", ".pwd");
        }
        Files.write(tempFile, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.TRUNCATE_EXISTING);
        tempFile.toFile().deleteOnExit();
        return tempFile;
    }

    public static void main(String[] args) throws Exception {
        // Prepare 5 test cases with secure temporary files
        Path f1 = createSecureTempFile("secret123");
        Path f2 = createSecureTempFile("secret123");
        Path f3 = createSecureTempFile("pass with spaces");
        Path f4 = createSecureTempFile("");
        Path f5 = createSecureTempFile("short");

        System.out.println(comparePasswordFromFile(f1.toString(), "secret123".toCharArray())); // true
        System.out.println(comparePasswordFromFile(f2.toString(), "secret124".toCharArray())); // false
        System.out.println(comparePasswordFromFile(f3.toString(), "pass with spaces".toCharArray())); // true
        System.out.println(comparePasswordFromFile(f4.toString(), "".toCharArray())); // true
        System.out.println(comparePasswordFromFile(f5.toString(), "shorter".toCharArray())); // false
    }
}