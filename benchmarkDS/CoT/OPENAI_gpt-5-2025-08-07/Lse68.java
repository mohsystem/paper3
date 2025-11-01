// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement upload_image that reads an image from a file path, converts to Base64, and inserts image name + Base64 into a database.
// 2) Security requirements: Validate image name, enforce max file size, safe file reading, avoid directory traversal misuse, use synchronized DB access.
// 3) Secure coding generation: Carefully validate inputs, handle exceptions, use immutable Base64 encoding, and clear resources.
// 4) Code review: Ensure bounds checks, thread-safety, and proper error handling; avoid leaking sensitive data.
// 5) Secure code output: Final code after fixing issues discovered in review.

import java.io.IOException;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.locks.ReentrantLock;

public class Lse68 {

    // Record to store in the database
    static final class ImageRecord {
        final String name;
        final String base64;
        ImageRecord(String name, String base64) {
            this.name = name;
            this.base64 = base64;
        }
    }

    // Simple thread-safe in-memory database
    static final class SafeDatabase {
        private final List<ImageRecord> store = new ArrayList<>();
        private final ReentrantLock lock = new ReentrantLock();

        public boolean insert(String name, String base64) {
            Objects.requireNonNull(name, "name");
            Objects.requireNonNull(base64, "base64");
            lock.lock();
            try {
                store.add(new ImageRecord(name, base64));
                return true;
            } finally {
                lock.unlock();
            }
        }

        public List<ImageRecord> getAll() {
            lock.lock();
            try {
                return new ArrayList<>(store);
            } finally {
                lock.unlock();
            }
        }

        public int count() {
            lock.lock();
            try {
                return store.size();
            } finally {
                lock.unlock();
            }
        }
    }

    private static boolean isValidName(String name) {
        if (name == null) return false;
        if (name.length() < 1 || name.length() > 100) return false;
        // Allow letters, digits, dot, underscore, dash
        return name.matches("[A-Za-z0-9._-]+");
    }

    private static byte[] readFileCapped(Path path, long maxBytes) throws IOException {
        if (!Files.exists(path) || !Files.isRegularFile(path)) {
            throw new IOException("File does not exist or is not a regular file");
        }
        long size = Files.size(path);
        if (size < 0 || size > maxBytes) {
            throw new IOException("File too large or invalid size");
        }
        return Files.readAllBytes(path);
    }

    // upload_image: reads an image from filePath, converts to Base64, inserts into db with imageName. Returns true if success.
    public static boolean upload_image(SafeDatabase db, String imageName, String filePath) {
        try {
            if (db == null) return false;
            if (!isValidName(imageName)) return false;

            Path p = Paths.get(filePath).toAbsolutePath().normalize();
            // Limit to 5 MiB
            long MAX_BYTES = 5L * 1024L * 1024L;
            byte[] data = readFileCapped(p, MAX_BYTES);

            String b64 = Base64.getEncoder().encodeToString(data);
            // Insert into the database
            return db.insert(imageName, b64);
        } catch (Exception ex) {
            return false;
        }
    }

    private static void writeBytesToFile(Path path, byte[] data) throws IOException {
        Files.write(path, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        SafeDatabase db = new SafeDatabase();

        Path tempDir = Files.createTempDirectory("lse68_java_imgs");
        // Create 5 test image files with pseudo data
        byte[][] testData = new byte[5][];
        for (int i = 0; i < 5; i++) {
            int size = 128 + i * 64; // small sizes
            byte[] arr = new byte[size];
            // Deterministic pseudo-content
            for (int j = 0; j < size; j++) {
                arr[j] = (byte) ((i * 37 + j) & 0xFF);
            }
            testData[i] = arr;
        }

        String[] names = {"img1.png", "photo-02.jpg", "scan_03.tif", "icon-4.bmp", "graph5.svg"};
        List<Path> paths = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            Path p = tempDir.resolve("test_" + (i + 1) + ".bin");
            writeBytesToFile(p, testData[i]);
            paths.add(p);
        }

        // 5 test cases
        for (int i = 0; i < 5; i++) {
            boolean ok = upload_image(db, names[i], paths.get(i).toString());
            System.out.println("Test " + (i + 1) + " inserted=" + ok);
        }

        System.out.println("Total records: " + db.count());
        // Optionally print first few chars of base64 to verify
        for (ImageRecord r : db.getAll()) {
            String preview = r.base64.substring(0, Math.min(20, r.base64.length()));
            System.out.println("Name=" + r.name + ", b64_preview=" + preview + "...");
        }
    }
}