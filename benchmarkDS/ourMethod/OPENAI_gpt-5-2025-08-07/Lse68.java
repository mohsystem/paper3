import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Base64;
import java.util.EnumSet;
import java.util.List;
import java.util.Objects;

public class Lse68 {
    private static final long MAX_IMAGE_SIZE = 1L * 1024 * 1024; // 1 MiB
    private static final int NAME_MAX = 255;

    static final class Record {
        final int id;
        final String name;
        final String b64;
        Record(int id, String name, String b64) {
            this.id = id;
            this.name = name;
            this.b64 = b64;
        }
    }

    static final class InMemoryDatabase {
        private final List<Record> records = new ArrayList<>();
        private int nextId = 1;

        public synchronized int insertImage(String name, String base64Data) {
            // Minimal validation at DB layer
            if (name == null || base64Data == null) throw new IllegalArgumentException("Invalid input");
            if (name.length() == 0 || name.length() > NAME_MAX) throw new IllegalArgumentException("Bad name");
            if (base64Data.length() == 0 || base64Data.length() > (int)(MAX_IMAGE_SIZE * 2)) throw new IllegalArgumentException("Data too large");
            int id = nextId++;
            records.add(new Record(id, name, base64Data));
            return id;
        }

        public synchronized int count() {
            return records.size();
        }

        public synchronized List<Record> all() {
            return new ArrayList<>(records);
        }
    }

    private static boolean isSafeSimpleName(String s) {
        if (s == null) return false;
        if (s.length() < 1 || s.length() > NAME_MAX) return false;
        // Only allow A-Z a-z 0-9 . _ -
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    // Securely read an image from baseDir/relativeFileName (simple filename), convert to Base64, and insert into db.
    public static boolean upload_image(Path baseDir, String relativeFileName, String imageName, InMemoryDatabase db) {
        Objects.requireNonNull(baseDir, "baseDir");
        Objects.requireNonNull(relativeFileName, "relativeFileName");
        Objects.requireNonNull(imageName, "imageName");
        Objects.requireNonNull(db, "db");

        if (!isSafeSimpleName(relativeFileName) || !isSafeSimpleName(imageName)) {
            System.out.println("Rejected: invalid name");
            return false;
        }

        try {
            // Resolve path within baseDir; require plain filename
            Path normalizedBase = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
            Path target = normalizedBase.resolve(relativeFileName).normalize();

            // Ensure target is within base dir
            if (!target.startsWith(normalizedBase)) {
                System.out.println("Rejected: path escapes base directory");
                return false;
            }

            // Open channel without following symlinks
            SeekableByteChannel channel = Files.newByteChannel(
                    target,
                    EnumSet.of(StandardOpenOption.READ),
                    LinkOption.NOFOLLOW_LINKS
            );

            try {
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                ByteBuffer buf = ByteBuffer.allocate(8192);
                long total = 0;
                while (true) {
                    buf.clear();
                    int n = channel.read(buf);
                    if (n < 0) break;
                    if (n == 0) continue;
                    total += n;
                    if (total > MAX_IMAGE_SIZE) {
                        System.out.println("Rejected: file too large");
                        return false;
                    }
                    buf.flip();
                    byte[] chunk = new byte[n];
                    buf.get(chunk);
                    baos.write(chunk);
                }
                byte[] data = baos.toByteArray();
                String b64 = Base64.getEncoder().encodeToString(data);
                db.insertImage(imageName, b64);
                return true;
            } finally {
                try { channel.close(); } catch (IOException ignored) {}
            }
        } catch (IOException | IllegalArgumentException ex) {
            System.out.println("Failed: " + ex.getMessage());
            return false;
        }
    }

    private static byte[] randomBytes(int n) {
        byte[] b = new byte[n];
        new SecureRandom().nextBytes(b);
        return b;
    }

    public static void main(String[] args) throws Exception {
        InMemoryDatabase db = new InMemoryDatabase();

        Path baseDir = Files.createTempDirectory("img_upload_base_");
        System.out.println("Base directory: " + baseDir.toString());

        // Prepare test files
        Files.write(baseDir.resolve("t1.bin"), randomBytes(1024));
        Files.write(baseDir.resolve("t2.bin"), randomBytes(2048));
        Files.write(baseDir.resolve("t3.bin"), randomBytes(512));

        // Oversize (2 MiB)
        ByteArrayOutputStream big = new ByteArrayOutputStream();
        byte[] chunk = randomBytes(65536);
        int toWrite = 2 * 1024 * 1024;
        while (big.size() < toWrite) {
            big.write(chunk, 0, Math.min(chunk.length, toWrite - big.size()));
        }
        Files.write(baseDir.resolve("big.bin"), big.toByteArray());

        // Non-existing file: t_missing.bin

        // 5 test cases
        boolean r1 = upload_image(baseDir, "t1.bin", "image_one.png", db);
        System.out.println("Test1 (valid): " + r1);

        boolean r2 = upload_image(baseDir, "t2.bin", "image-two.jpg", db);
        System.out.println("Test2 (valid): " + r2);

        boolean r3 = upload_image(baseDir, "big.bin", "too_big.dat", db);
        System.out.println("Test3 (oversize): " + r3);

        boolean r4 = upload_image(baseDir, "t_missing.bin", "missing.gif", db);
        System.out.println("Test4 (missing): " + r4);

        boolean r5 = upload_image(baseDir, "t3.bin", "safe_name-ok_123.bmp", db);
        System.out.println("Test5 (valid): " + r5);

        System.out.println("DB count: " + db.count());
        for (Record rec : db.all()) {
            System.out.println("Record " + rec.id + " name=" + rec.name + " b64_len=" + rec.b64.length());
        }

        // Cleanup attempt (best-effort)
        try {
            Files.deleteIfExists(baseDir.resolve("t1.bin"));
            Files.deleteIfExists(baseDir.resolve("t2.bin"));
            Files.deleteIfExists(baseDir.resolve("t3.bin"));
            Files.deleteIfExists(baseDir.resolve("big.bin"));
            Files.deleteIfExists(baseDir);
        } catch (Exception ignored) {}
    }
}