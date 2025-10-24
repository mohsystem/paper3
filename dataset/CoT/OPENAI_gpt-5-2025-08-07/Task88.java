import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.util.*;

/*
Chain-of-Through in code (comments only):
1) Problem understanding:
   - Create a temp file (UTF-8), make a random string, convert it to Unicode code point notation, write to file, return temp path.
2) Security requirements:
   - Use SecureRandom, safe temp file creation, restrict permissions where supported (POSIX).
3) Secure coding generation:
   - Validate inputs, avoid unsafe APIs, handle exceptions.
4) Code review:
   - Use try-with-resources, avoid leaking file handles, handle platform differences for permissions.
5) Secure code output:
   - Final function produces path; main runs 5 tests.
*/

public class Task88 {

    // Predefined Unicode code points (include ASCII, extended, and a few supplementary)
    private static final int[] CODE_POINTS = buildCodePointsPool();

    private static int[] buildCodePointsPool() {
        List<Integer> cps = new ArrayList<>();
        // ASCII letters and digits
        for (int c = 'A'; c <= 'Z'; c++) cps.add(c);
        for (int c = 'a'; c <= 'z'; c++) cps.add(c);
        for (int c = '0'; c <= '9'; c++) cps.add(c);
        // Some extended BMP characters
        int[] extra = {
            0x00E9, // é
            0x03A9, // Ω
            0x0416, // Ж
            0x4F60, // 你
            0x597D, // 好
            0x6F22, // 漢
            0x20AC, // €
            0x2603, // ☃
        };
        for (int cp : extra) cps.add(cp);
        // Some supplementary characters (require surrogate pairs)
        int[] supplementary = {
            0x1F642, // 🙂
            0x1F680, // 🚀
            0x1F9E9, // 🧩
            0x1D6D1  // 𝛑
        };
        for (int cp : supplementary) cps.add(cp);
        // Convert to primitive array
        int[] arr = new int[cps.size()];
        for (int i = 0; i < cps.size(); i++) arr[i] = cps.get(i);
        return arr;
    }

    // Public API: creates a UTF-8 temp file with the Unicode code point representation of a random string; returns path
    public static String createTempUnicodeFile(int length) throws IOException {
        int safeLen = sanitizeLength(length);
        String random = generateRandomString(safeLen);
        String unicode = toUnicodeCodePoints(random);
        Path path = createSecureTempFile();
        writeUtf8(path, unicode);
        return path.toString();
    }

    private static int sanitizeLength(int length) {
        if (length < 0) return 16;
        int max = 4096;
        return Math.min(length, max);
    }

    private static String generateRandomString(int length) {
        SecureRandom rng = new SecureRandom();
        StringBuilder sb = new StringBuilder(length * 2);
        for (int i = 0; i < length; i++) {
            int cp = CODE_POINTS[rng.nextInt(CODE_POINTS.length)];
            sb.appendCodePoint(cp);
        }
        return sb.toString();
    }

    private static String toUnicodeCodePoints(String s) {
        StringBuilder sb = new StringBuilder(s.length() * 7);
        s.codePoints().forEach(cp -> {
            if (sb.length() > 0) sb.append(' ');
            if (cp <= 0xFFFF) {
                sb.append(String.format("U+%04X", cp));
            } else {
                sb.append(String.format("U+%06X", cp));
            }
        });
        return sb.toString();
    }

    private static Path createSecureTempFile() throws IOException {
        Path p = Files.createTempFile("task88_", ".txt");
        // Attempt to set restrictive permissions on POSIX systems
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            Files.setPosixFilePermissions(p, perms);
        } catch (UnsupportedOperationException ignored) {
            // Not POSIX or not supported; ignore.
        }
        return p;
    }

    private static void writeUtf8(Path path, String content) throws IOException {
        try (BufferedWriter bw = Files.newBufferedWriter(path, StandardCharsets.UTF_8,
                StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
            bw.write(content);
        }
    }

    // Test harness with 5 cases
    public static void main(String[] args) {
        int[] tests = {0, 1, 5, 10, 50};
        for (int t : tests) {
            try {
                String path = createTempUnicodeFile(t);
                System.out.println("Created temp file (len=" + t + "): " + path);
            } catch (IOException e) {
                System.out.println("Failed for len=" + t + ": " + e.getMessage());
            }
        }
    }
}