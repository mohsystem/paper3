import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.EnumSet;
import java.util.Set;

public class Task88 {

    private static final String ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

    public static String createTempUnicodeFile(int length) {
        if (length <= 0) {
            throw new IllegalArgumentException("length must be > 0");
        }

        SecureRandom sr;
        try {
            sr = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            sr = new SecureRandom();
        }

        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int idx = sr.nextInt(ALPHABET.length());
            sb.append(ALPHABET.charAt(idx));
        }

        String random = sb.toString();

        StringBuilder unicodeEscaped = new StringBuilder(random.length() * 6);
        for (int i = 0; i < random.length(); i++) {
            char ch = random.charAt(i);
            unicodeEscaped.append(String.format("\\u%04X", (int) ch));
        }

        try {
            Path temp = Files.createTempFile("task88_", ".txt");

            // Restrict permissions on POSIX systems if supported
            try {
                PosixFileAttributeView view = Files.getFileAttributeView(temp, PosixFileAttributeView.class);
                if (view != null) {
                    Set<PosixFilePermission> perms = EnumSet.of(
                            PosixFilePermission.OWNER_READ,
                            PosixFilePermission.OWNER_WRITE
                    );
                    Files.setPosixFilePermissions(temp, perms);
                }
            } catch (UnsupportedOperationException ignore) {
                // Non-POSIX or not supported; ignore
            }

            try (java.io.BufferedWriter writer = Files.newBufferedWriter(temp, StandardCharsets.UTF_8,
                    StandardOpenOption.WRITE)) {
                writer.write(unicodeEscaped.toString());
                writer.flush();
            }
            return temp.toAbsolutePath().toString();
        } catch (IOException e) {
            throw new UncheckedIOException("Failed to create/write temporary file", e);
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(createTempUnicodeFile(8));
        System.out.println(createTempUnicodeFile(12));
        System.out.println(createTempUnicodeFile(1));
        System.out.println(createTempUnicodeFile(32));
        System.out.println(createTempUnicodeFile(64));
    }
}