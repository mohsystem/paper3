import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.LinkedHashMap;
import java.util.Objects;
import java.util.Queue;
import java.util.concurrent.*;
import java.util.stream.Collectors;

public class Task0 {

    public static Map<String, String> processDirectory(String baseDir, int workers) throws IOException {
        Objects.requireNonNull(baseDir, "baseDir");
        if (workers <= 0 || workers > 128) {
            throw new IllegalArgumentException("workers must be in 1..128");
        }

        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        if (!Files.exists(base, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Base directory does not exist");
        }
        if (Files.isSymbolicLink(base)) {
            throw new IOException("Base directory must not be a symlink");
        }
        if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Base path is not a directory");
        }
        Path baseReal = base.toRealPath(LinkOption.NOFOLLOW_LINKS);

        List<Path> files = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(baseReal)) {
            for (Path p : stream) {
                try {
                    if (Files.isSymbolicLink(p)) continue;
                    BasicFileAttributes attrs = Files.readAttributes(p, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
                    if (!attrs.isRegularFile()) continue;
                    Path real = p.toRealPath(LinkOption.NOFOLLOW_LINKS);
                    if (!real.startsWith(baseReal)) continue; // ensure within base
                    files.add(real);
                } catch (IOException ignored) {
                    // skip entries we cannot safely process
                }
            }
        }

        // Use a thread-safe queue of files
        Queue<Path> queue = new ConcurrentLinkedQueue<>(files);
        ConcurrentMap<String, String> results = new ConcurrentHashMap<>();

        ExecutorService pool = Executors.newFixedThreadPool(workers);
        List<Future<?>> futures = new ArrayList<>();
        for (int i = 0; i < workers; i++) {
            futures.add(pool.submit(() -> {
                Path p;
                while ((p = queue.poll()) != null) {
                    // Double-check that file is still within base and is not a symlink
                    try {
                        if (Files.isSymbolicLink(p)) continue;
                        Path real = p.toRealPath(LinkOption.NOFOLLOW_LINKS);
                        if (!real.startsWith(baseReal)) continue;

                        // Lock the file exclusively to ensure only one thread processes it at a time
                        try (FileChannel channel = FileChannel.open(real, StandardOpenOption.READ, StandardOpenOption.WRITE)) {
                            try (FileLock lock = channel.lock(0L, Long.MAX_VALUE, false)) {
                                long fnv64 = 0xcbf29ce484222325L; // FNV-1a offset
                                long prime = 0x100000001b3L;
                                ByteBuffer buf = ByteBuffer.allocate(8192);
                                channel.position(0);
                                while (true) {
                                    buf.clear();
                                    int read = channel.read(buf);
                                    if (read == -1) break;
                                    buf.flip();
                                    for (int idx = 0; idx < buf.limit(); idx++) {
                                        int b = buf.get(idx) & 0xFF;
                                        fnv64 ^= (long) b;
                                        fnv64 *= prime;
                                    }
                                }
                                String name = real.getFileName().toString();
                                String hex = toUnsignedHex(fnv64);
                                results.put(name, hex);
                            }
                        } catch (IOException e) {
                            results.put(p.getFileName().toString(), "ERROR");
                        }
                    } catch (IOException e) {
                        results.put(p.getFileName().toString(), "ERROR");
                    }
                }
            }));
        }

        for (Future<?> f : futures) {
            try {
                f.get();
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
            } catch (ExecutionException ee) {
                // ignore worker exception; results may contain errors
            }
        }
        pool.shutdown();

        // Return results sorted by filename for determinism
        return results.entrySet().stream()
                .sorted(Map.Entry.comparingByKey())
                .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue,
                        (a, b) -> a, LinkedHashMap::new));
    }

    private static String toUnsignedHex(long value) {
        // Represent as 16-character zero-padded unsigned hex
        String s = Long.toHexString(value);
        if (s.length() < 16) {
            StringBuilder sb = new StringBuilder(16);
            for (int i = 0; i < 16 - s.length(); i++) sb.append('0');
            sb.append(s);
            return sb.toString();
        } else if (s.length() > 16) {
            return s.substring(s.length() - 16);
        }
        return s;
        // Note: Java's toHexString uses two's complement; length handling ensures 16 chars.
    }

    private static void writeFile(Path dir, String name, String content) throws IOException {
        Path p = dir.resolve(name).normalize();
        if (!p.startsWith(dir)) throw new IOException("Invalid file name");
        try (FileChannel ch = FileChannel.open(p, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
            byte[] data = content.getBytes(StandardCharsets.UTF_8);
            ByteBuffer bb = ByteBuffer.wrap(data);
            while (bb.hasRemaining()) ch.write(bb);
            ch.force(true);
        }
    }

    public static void main(String[] args) throws Exception {
        Path tmp = Files.createTempDirectory("concurrent_proc_java").toRealPath(LinkOption.NOFOLLOW_LINKS);
        try {
            writeFile(tmp, "file1.txt", "Alpha\n".repeat(10000));
            writeFile(tmp, "file2.txt", "Bravo\n".repeat(8000));
            writeFile(tmp, "file3.txt", "Charlie\n".repeat(12000));
            writeFile(tmp, "file4.txt", "Delta\n".repeat(5000));
            writeFile(tmp, "file5.txt", "Echo\n".repeat(15000));

            Map<String, String> result = processDirectory(tmp.toString(), 4);
            System.out.println("Processed files and FNV-1a(64) hashes:");
            for (Map.Entry<String, String> e : result.entrySet()) {
                System.out.println(e.getKey() + " -> " + e.getValue());
            }
        } finally {
            // Cleanup best-effort
            try (DirectoryStream<Path> ds = Files.newDirectoryStream(tmp)) {
                for (Path p : ds) {
                    try { Files.deleteIfExists(p); } catch (Exception ignored) {}
                }
            } catch (Exception ignored) {}
            try { Files.deleteIfExists(tmp); } catch (Exception ignored) {}
        }
    }
}