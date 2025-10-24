import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.util.EnumSet;
import java.util.Set;

public class Task88 {
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    // Ranges: digits, uppercase, lowercase, Latin-1 supplement, Cyrillic, Emoji
    private static final int[][] RANGES = new int[][] {
        {0x0030, 0x0039}, {0x0041, 0x005A}, {0x0061, 0x007A},
        {0x00C0, 0x00FF}, {0x0400, 0x04FF}, {0x1F600, 0x1F64F}
    };

    public static String createTempUnicodeFile(int length) throws IOException {
        if (length < 1 || length > 1024) {
            throw new IllegalArgumentException("length must be in [1, 1024]");
        }

        String randomString = generateRandomUnicodeString(length);
        String codepoints = toCodepointsLine(randomString);

        Path tempFile = Files.createTempFile("task88_", ".txt");
        // Set restrictive permissions where supported (POSIX)
        try {
            Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
            Files.setPosixFilePermissions(tempFile, perms);
        } catch (UnsupportedOperationException ignored) {
            // Non-POSIX file systems (e.g., Windows) do not support POSIX permissions
        }

        try (FileOutputStream fos = new FileOutputStream(tempFile.toFile(), false);
             OutputStreamWriter osw = new OutputStreamWriter(fos, StandardCharsets.UTF_8);
             BufferedWriter bw = new BufferedWriter(osw)) {
            bw.write("Original: ");
            bw.write(randomString);
            bw.newLine();
            bw.write("Codepoints: ");
            bw.write(codepoints);
            bw.newLine();
            bw.flush();
            fos.getChannel().force(true); // fsync to mitigate TOCTOU/power-loss issues
        }

        return tempFile.toAbsolutePath().toString();
    }

    private static String generateRandomUnicodeString(int length) {
        StringBuilder sb = new StringBuilder(length * 2);
        int produced = 0;
        int attempts = 0;
        int maxAttempts = length * 50;
        while (produced < length && attempts < maxAttempts) {
            attempts++;
            int[] r = RANGES[SECURE_RANDOM.nextInt(RANGES.length)];
            int cp = r[0] + SECURE_RANDOM.nextInt(r[1] - r[0] + 1);
            if (cp >= 0xD800 && cp <= 0xDFFF) {
                continue; // skip surrogate range
            }
            if (!Character.isDefined(cp)) {
                continue;
            }
            sb.appendCodePoint(cp);
            produced++;
        }
        if (produced < length) {
            throw new IllegalStateException("Failed to generate sufficient random Unicode characters");
        }
        return sb.toString();
    }

    private static String toCodepointsLine(String s) {
        StringBuilder u = new StringBuilder(s.length() * 7);
        s.codePoints().forEach(cp -> {
            String hex = Integer.toHexString(cp).toUpperCase();
            int pad = (cp <= 0xFFFF) ? 4 : 6;
            while (hex.length() < pad) hex = "0" + hex;
            u.append("U+").append(hex).append(' ');
        });
        if (u.length() > 0) u.setLength(u.length() - 1); // trim trailing space
        return u.toString();
    }

    public static void main(String[] args) {
        int[] tests = {8, 16, 1, 32, 50};
        for (int t : tests) {
            try {
                String path = createTempUnicodeFile(t);
                System.out.println("Created file (length " + t + "): " + path);
            } catch (Exception e) {
                System.out.println("Error for length " + t + ": " + e.getMessage());
            }
        }
    }
}