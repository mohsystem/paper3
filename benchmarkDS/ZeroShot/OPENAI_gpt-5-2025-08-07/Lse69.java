import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Objects;

public class Lse69 {

    private static final long MAX_SIZE = 10L * 1024 * 1024; // 10 MB limit
    private static final String BASE_DIR = "out_java";

    private static String sanitizeFileName(String name) {
        if (name == null) throw new IllegalArgumentException("Filename is null");
        if (name.length() == 0 || name.length() > 255) {
            throw new IllegalArgumentException("Invalid filename length");
        }
        for (int i = 0; i < name.length(); i++) {
            char c = name.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '_' || c == '-';
            if (!ok) {
                throw new IllegalArgumentException("Invalid character in filename");
            }
        }
        if (name.equals(".") || name.equals("..")) {
            throw new IllegalArgumentException("Invalid filename");
        }
        return name;
    }

    // Returns number of bytes written to the encoded file
    public static long upload_image(byte[] imageData, String outputFileName) throws IOException {
        Objects.requireNonNull(imageData, "imageData is null");
        if (imageData.length > MAX_SIZE) {
            throw new IllegalArgumentException("Image too large");
        }
        String safeName = sanitizeFileName(outputFileName);

        Path base = Paths.get(BASE_DIR).toAbsolutePath().normalize();
        Files.createDirectories(base);
        Path out = base.resolve(safeName).normalize();
        if (!out.startsWith(base)) {
            throw new SecurityException("Path traversal detected");
        }

        long encodedLen = ((imageData.length + 2L) / 3L) * 4L;

        Base64.Encoder encoder = Base64.getEncoder();
        try (OutputStream fos = Files.newOutputStream(out, StandardOpenOption.CREATE_NEW);
             BufferedOutputStream bos = new BufferedOutputStream(fos);
             OutputStream b64 = encoder.wrap(bos);
             ByteArrayInputStream in = new ByteArrayInputStream(imageData)) {

            byte[] buf = new byte[8192];
            int r;
            while ((r = in.read(buf)) != -1) {
                b64.write(buf, 0, r);
            }
        }
        return encodedLen;
    }

    private static byte[] randomBytes(int n) {
        byte[] b = new byte[n];
        new SecureRandom().nextBytes(b);
        return b;
    }

    public static void main(String[] args) {
        try {
            System.out.println("Test1: " + upload_image(randomBytes(0), "img0.b64") + " bytes written.");
        } catch (Exception e) { System.out.println("Test1 failed: " + e.getMessage()); }

        try {
            System.out.println("Test2: " + upload_image(randomBytes(10), "img1.b64") + " bytes written.");
        } catch (Exception e) { System.out.println("Test2 failed: " + e.getMessage()); }

        try {
            System.out.println("Test3: " + upload_image(randomBytes(1024), "img2.b64") + " bytes written.");
        } catch (Exception e) { System.out.println("Test3 failed: " + e.getMessage()); }

        try {
            System.out.println("Test4: " + upload_image(randomBytes(2048), "img3.b64") + " bytes written.");
        } catch (Exception e) { System.out.println("Test4 failed: " + e.getMessage()); }

        try {
            System.out.println("Test5: " + upload_image(randomBytes(9999), "img4.b64") + " bytes written.");
        } catch (Exception e) { System.out.println("Test5 failed: " + e.getMessage()); }
    }
}