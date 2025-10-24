import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;

public class Task0 {

    public static Map<String, Long> processDirectory(String dirPath, int threadCount) throws Exception {
        Path dir = Paths.get(dirPath);
        if (!Files.isDirectory(dir)) {
            throw new IllegalArgumentException("Not a directory: " + dirPath);
        }

        List<Path> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(dir)) {
            for (Path p : stream) {
                if (Files.isRegularFile(p)) {
                    files.add(p);
                }
            }
        }

        ConcurrentLinkedQueue<Path> queue = new ConcurrentLinkedQueue<>(files);
        ConcurrentHashMap<String, Long> results = new ConcurrentHashMap<>();

        ExecutorService pool = Executors.newFixedThreadPool(Math.max(1, threadCount));
        List<Future<?>> futures = new ArrayList<>();

        for (int i = 0; i < threadCount; i++) {
            futures.add(pool.submit(() -> {
                byte[] buf = new byte[8192];
                while (true) {
                    Path p = queue.poll();
                    if (p == null) break;
                    long sum = 0L;
                    try (InputStream in = Files.newInputStream(p)) {
                        int r;
                        while ((r = in.read(buf)) != -1) {
                            for (int k = 0; k < r; k++) {
                                sum += (buf[k] & 0xFF);
                            }
                        }
                        results.put(p.getFileName().toString(), sum);
                    } catch (IOException e) {
                        results.put(p.getFileName().toString(), -1L);
                    }
                }
            }));
        }

        for (Future<?> f : futures) f.get();
        pool.shutdown();
        return results;
    }

    private static Path createTestDirWithFiles(String prefix, int fileCount, int minKB, int maxKB) throws Exception {
        Path dir = Files.createTempDirectory(prefix);
        Random rnd = new Random();
        for (int i = 0; i < fileCount; i++) {
            int sizeKB = minKB + rnd.nextInt(Math.max(1, (maxKB - minKB + 1)));
            Path f = dir.resolve("file_" + i + ".dat");
            try (OutputStream out = Files.newOutputStream(f)) {
                byte[] buf = new byte[8192];
                int bytesToWrite = sizeKB * 1024;
                while (bytesToWrite > 0) {
                    rnd.nextBytes(buf);
                    int chunk = Math.min(bytesToWrite, buf.length);
                    out.write(buf, 0, chunk);
                    bytesToWrite -= chunk;
                }
            }
        }
        return dir;
    }

    private static void runTestCase(String name, int files, int threads) throws Exception {
        Path dir = createTestDirWithFiles(name + "_", files, 32, 128);
        System.out.println("Test " + name + " - dir: " + dir + " threads: " + threads);
        Map<String, Long> res = processDirectory(dir.toString(), threads);
        List<String> keys = new ArrayList<>(res.keySet());
        Collections.sort(keys);
        for (String k : keys) {
            System.out.println("  " + k + " -> checksum=" + res.get(k));
        }
        System.out.println();
    }

    public static void main(String[] args) throws Exception {
        runTestCase("case1", 5, 1);
        runTestCase("case2", 6, 2);
        runTestCase("case3", 7, 3);
        runTestCase("case4", 8, 4);
        runTestCase("case5", 9, 5);
    }
}