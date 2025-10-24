import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.*;
import java.util.concurrent.*;

public class Task0 {

    // Secure, concurrent processing of files in a directory.
    // Returns a map of file absolute path -> SHA-256 hex digest.
    public static Map<String, String> processDirectory(String dirPath, int threadCount) throws IOException, NoSuchAlgorithmException, InterruptedException {
        if (dirPath == null) {
            throw new IllegalArgumentException("Directory path cannot be null");
        }
        Path dir = Paths.get(dirPath).toAbsolutePath().normalize();
        if (!Files.exists(dir) || !Files.isDirectory(dir, LinkOption.NOFOLLOW_LINKS)) {
            throw new IllegalArgumentException("Path is not a directory: " + dir);
        }
        // Avoid processing a symlinked directory to reduce risk
        if (Files.isSymbolicLink(dir)) {
            throw new IllegalArgumentException("Refusing to process a symlinked directory: " + dir);
        }

        List<Path> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(dir)) {
            for (Path p : stream) {
                try {
                    if (!Files.isSymbolicLink(p) && Files.isRegularFile(p, LinkOption.NOFOLLOW_LINKS)) {
                        files.add(p.toAbsolutePath().normalize());
                    }
                } catch (SecurityException se) {
                    // Skip entries we cannot access
                }
            }
        }

        int maxThreads = Math.max(1, Math.min(threadCount, Runtime.getRuntime().availableProcessors() * 4));
        ConcurrentMap<String, String> results = new ConcurrentHashMap<>();
        AtomicInteger index = new AtomicInteger(0);

        ExecutorService pool = Executors.newFixedThreadPool(maxThreads, r -> {
            Thread t = new Thread(r);
            t.setDaemon(true);
            t.setName("worker-" + t.getId());
            return t;
        });

        List<Future<?>> futures = new ArrayList<>();
        for (int i = 0; i < maxThreads; i++) {
            futures.add(pool.submit(() -> {
                try {
                    MessageDigest md = MessageDigest.getInstance("SHA-256");
                    ByteBuffer buffer = ByteBuffer.allocateDirect(64 * 1024);
                    while (true) {
                        int i1 = index.getAndIncrement();
                        if (i1 >= files.size()) break;
                        Path file = files.get(i1);
                        // Compute digest using streaming
                        md.reset();
                        try (SeekableByteChannel ch = Files.newByteChannel(file, StandardOpenOption.READ)) {
                            buffer.clear();
                            while (ch.read(buffer) > 0) {
                                buffer.flip();
                                byte[] chunk = new byte[buffer.remaining()];
                                buffer.get(chunk);
                                md.update(chunk);
                                buffer.clear();
                            }
                        } catch (IOException ioe) {
                            // Record an identifiable error marker per file
                            results.put(file.toString(), "ERROR:" + ioe.getClass().getSimpleName());
                            continue;
                        }
                        byte[] digest = md.digest();
                        results.put(file.toString(), toHex(digest));
                    }
                } catch (NoSuchAlgorithmException e) {
                    // Should not happen for SHA-256; if it does, propagate as unchecked
                    throw new RuntimeException(e);
                }
            }));
        }

        pool.shutdown();
        for (Future<?> f : futures) {
            try {
                f.get();
            } catch (ExecutionException ee) {
                // Propagate any worker exception
                throw new RuntimeException(ee.getCause());
            }
        }
        if (!pool.awaitTermination(5, TimeUnit.MINUTES)) {
            pool.shutdownNow();
            throw new RuntimeException("Timeout waiting for tasks");
        }

        // Convert to HashMap for return
        return new HashMap<>(results);
    }

    private static String toHex(byte[] data) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[data.length * 2];
        for (int j = 0; j < data.length; j++) {
            int v = data[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    // Helpers for tests
    private static Path createTempDir(String prefix) throws IOException {
        return Files.createTempDirectory(prefix).toAbsolutePath().normalize();
    }

    private static void writeFile(Path dir, String name, byte[] content) throws IOException {
        Path p = dir.resolve(name).normalize();
        if (!p.startsWith(dir)) { // basic traversal defense
            throw new SecurityException("Invalid file path");
        }
        try (OutputStream os = Files.newOutputStream(p, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
            os.write(content);
        }
    }

    private static void populateFiles(Path dir, int count, String seed) throws IOException {
        for (int i = 0; i < count; i++) {
            String name = String.format("file_%02d.dat", i);
            byte[] content = ("seed:" + seed + "; index:" + i + "; data:" + "X".repeat((i % 17) + 1)).getBytes();
            writeFile(dir, name, content);
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        // 1) Empty directory
        Path d1 = createTempDir("cfp1_");
        Map<String, String> r1 = processDirectory(d1.toString(), 4);
        System.out.println("Test1 files=" + r1.size());

        // 2) Small set, 2 threads
        Path d2 = createTempDir("cfp2_");
        populateFiles(d2, 5, "A");
        Map<String, String> r2 = processDirectory(d2.toString(), 2);
        System.out.println("Test2 files=" + r2.size());

        // 3) Larger set, 4 threads
        Path d3 = createTempDir("cfp3_");
        populateFiles(d3, 20, "B");
        Map<String, String> r3 = processDirectory(d3.toString(), 4);
        System.out.println("Test3 files=" + r3.size());

        // 4) Larger set, 8 threads
        Path d4 = createTempDir("cfp4_");
        populateFiles(d4, 50, "C");
        Map<String, String> r4 = processDirectory(d4.toString(), 8);
        System.out.println("Test4 files=" + r4.size());

        // 5) Single-threaded run
        Path d5 = createTempDir("cfp5_");
        populateFiles(d5, 10, "D");
        Map<String, String> r5 = processDirectory(d5.toString(), 1);
        System.out.println("Test5 files=" + r5.size());
    }
}