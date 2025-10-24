import java.io.*;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.*;
import java.util.stream.Collectors;

public class Task0 {

    // Secure FNV-1a 64-bit hash for streaming
    private static String fnv1a64Hex(Path file) throws IOException {
        final long FNV_OFFSET_BASIS = 0xcbf29ce484222325L;
        final long FNV_PRIME = 0x100000001b3L;
        long hash = FNV_OFFSET_BASIS;

        try (InputStream is = new BufferedInputStream(Files.newInputStream(file))) {
            byte[] buffer = new byte[64 * 1024];
            int read;
            while ((read = is.read(buffer)) != -1) {
                for (int i = 0; i < read; i++) {
                    hash ^= (buffer[i] & 0xFF);
                    hash *= FNV_PRIME;
                }
            }
        }
        // 16 hex digits zero-padded
        return String.format("%016x", hash);
    }

    // Securely list regular files in a directory (non-recursive)
    private static List<Path> listRegularFiles(Path dir) throws IOException {
        if (!Files.isDirectory(dir)) {
            return Collections.emptyList();
        }
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(dir, entry -> Files.isRegularFile(entry))) {
            List<Path> files = new ArrayList<>();
            for (Path p : stream) files.add(p);
            return files;
        }
    }

    // Core concurrent processing: returns map of filename -> hash
    public static Map<String, String> processDirectory(String dirPath, int threadCount) throws IOException, InterruptedException {
        if (threadCount <= 0) throw new IllegalArgumentException("threadCount must be > 0");
        Path dir = Paths.get(dirPath).toAbsolutePath().normalize();
        List<Path> files = listRegularFiles(dir);

        ConcurrentLinkedQueue<Path> queue = new ConcurrentLinkedQueue<>(files);
        ConcurrentHashMap<String, String> results = new ConcurrentHashMap<>();

        ExecutorService pool = Executors.newFixedThreadPool(threadCount, r -> {
            Thread t = new Thread(r);
            t.setDaemon(true);
            t.setName("worker-" + t.getId());
            return t;
        });
        List<Future<?>> futures = new ArrayList<>();
        int workers = Math.min(threadCount, Math.max(1, files.size()));
        for (int i = 0; i < workers; i++) {
            futures.add(pool.submit(() -> {
                try {
                    Path f;
                    while ((f = queue.poll()) != null) {
                        String hex = fnv1a64Hex(f);
                        results.put(f.getFileName().toString(), hex);
                    }
                } catch (IOException e) {
                    // Log to stderr without halting other workers
                    System.err.println("Error processing file: " + e.getMessage());
                }
            }));
        }
        for (Future<?> fut : futures) {
            try {
                fut.get();
            } catch (ExecutionException e) {
                System.err.println("Worker error: " + e.getCause());
            }
        }
        pool.shutdown();
        pool.awaitTermination(1, TimeUnit.MINUTES);

        return results;
    }

    // Helper to write a file of a given size with a pattern, streaming to avoid memory pressure
    private static void writeFile(Path file, long sizeBytes, byte seed) throws IOException {
        Files.createDirectories(file.getParent());
        try (OutputStream os = new BufferedOutputStream(Files.newOutputStream(file, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING))) {
            byte[] chunk = new byte[8192];
            Arrays.fill(chunk, seed);
            long remaining = sizeBytes;
            while (remaining > 0) {
                int toWrite = (int) Math.min(chunk.length, remaining);
                os.write(chunk, 0, toWrite);
                remaining -= toWrite;
                seed++; // slight variation
                if (toWrite == chunk.length) {
                    Arrays.fill(chunk, seed);
                }
            }
        }
    }

    private static Path createTestDir(String baseName) throws IOException {
        Path tmp = Paths.get(System.getProperty("java.io.tmpdir")).toAbsolutePath();
        Path dir = tmp.resolve(baseName + "-" + UUID.randomUUID());
        Files.createDirectories(dir);
        return dir;
    }

    private static void printResults(String title, Map<String, String> results) {
        System.out.println("=== " + title + " ===");
        List<String> keys = new ArrayList<>(results.keySet());
        Collections.sort(keys);
        for (String k : keys) {
            System.out.println(k + " -> " + results.get(k));
        }
        System.out.println();
    }

    public static void main(String[] args) throws Exception {
        // 5 Test Cases
        // Test 1: 3 small files, 1 thread
        Path dir1 = createTestDir("task0-tc1");
        writeFile(dir1.resolve("a.txt"), 1024, (byte) 1);
        writeFile(dir1.resolve("b.txt"), 2048, (byte) 2);
        writeFile(dir1.resolve("c.txt"), 4096, (byte) 3);
        Map<String, String> r1 = processDirectory(dir1.toString(), 1);
        printResults("Test 1 (3 files, 1 thread)", r1);

        // Test 2: 10 varied files, 2 threads
        Path dir2 = createTestDir("task0-tc2");
        for (int i = 0; i < 10; i++) {
            writeFile(dir2.resolve("file" + i + ".bin"), (i + 1) * 1500, (byte) (10 + i));
        }
        Map<String, String> r2 = processDirectory(dir2.toString(), 2);
        printResults("Test 2 (10 files, 2 threads)", r2);

        // Test 3: 5 files incl empty, 4 threads
        Path dir3 = createTestDir("task0-tc3");
        writeFile(dir3.resolve("empty.dat"), 0, (byte) 0);
        for (int i = 0; i < 4; i++) {
            writeFile(dir3.resolve("d" + i + ".dat"), (i + 1) * 8192, (byte) (20 + i));
        }
        Map<String, String> r3 = processDirectory(dir3.toString(), 4);
        printResults("Test 3 (5 files incl empty, 4 threads)", r3);

        // Test 4: empty directory, 3 threads
        Path dir4 = createTestDir("task0-tc4");
        Map<String, String> r4 = processDirectory(dir4.toString(), 3);
        printResults("Test 4 (0 files, 3 threads)", r4);

        // Test 5: 20 larger files, 8 threads
        Path dir5 = createTestDir("task0-tc5");
        SecureRandom random = new SecureRandom();
        for (int i = 0; i < 20; i++) {
            long size = 200 * 1024 + random.nextInt(50 * 1024); // ~200KB-250KB
            writeFile(dir5.resolve(String.format("large_%02d.bin", i)), size, (byte) (30 + i));
        }
        Map<String, String> r5 = processDirectory(dir5.toString(), 8);
        printResults("Test 5 (20 larger files, 8 threads)", r5);
    }
}