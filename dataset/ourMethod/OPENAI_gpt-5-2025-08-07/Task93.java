import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

public class Task93 {

    public static final class Pair {
        public final String key;
        public final String value;

        public Pair(String k, String v) {
            this.key = k;
            this.value = v;
        }

        @Override
        public String toString() {
            return key + "=" + value;
        }
    }

    public static List<Pair> readAndSortKeyValueFile(Path baseDir, String relativePath) throws IOException {
        if (baseDir == null) {
            throw new IllegalArgumentException("Base directory must not be null.");
        }
        if (relativePath == null || relativePath.length() == 0 || relativePath.length() > 4096) {
            throw new IllegalArgumentException("Invalid relative path.");
        }
        if (relativePath.indexOf('\0') >= 0) {
            throw new IllegalArgumentException("Path contains NUL byte.");
        }

        Path baseReal = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path rel = Paths.get(relativePath);
        if (rel.isAbsolute()) {
            throw new SecurityException("Absolute paths are not allowed.");
        }
        Path target = baseReal.resolve(rel).normalize();

        if (!target.startsWith(baseReal)) {
            throw new SecurityException("Resolved path escapes base directory.");
        }
        if (!Files.exists(target, LinkOption.NOFOLLOW_LINKS)) {
            throw new NoSuchFileException("File not found: " + target);
        }
        if (Files.isSymbolicLink(target)) {
            throw new SecurityException("Refusing to process symlink.");
        }
        if (!Files.isRegularFile(target, LinkOption.NOFOLLOW_LINKS)) {
            throw new SecurityException("Not a regular file.");
        }

        List<Pair> records = new ArrayList<>();
        try (BufferedReader br = Files.newBufferedReader(target, StandardCharsets.UTF_8)) {
            String line;
            int lineNo = 0;
            while ((line = br.readLine()) != null) {
                lineNo++;
                if (line.length() > 8192) {
                    // Skip unreasonably long lines
                    continue;
                }
                String trimmed = line.trim();
                if (trimmed.isEmpty() || trimmed.startsWith("#") || trimmed.startsWith(";")) {
                    continue;
                }
                int idx = trimmed.indexOf('=');
                if (idx <= 0 || idx == trimmed.length() - 1) {
                    // malformed: no key or no value
                    continue;
                }
                String key = trimmed.substring(0, idx).trim();
                String val = trimmed.substring(idx + 1).trim();
                if (key.isEmpty() || val.isEmpty() || key.length() > 1024 || val.length() > 4096) {
                    continue;
                }
                records.add(new Pair(key, val));
            }
        }

        records.sort(new Comparator<Pair>() {
            @Override
            public int compare(Pair a, Pair b) {
                int c = a.key.compareTo(b.key);
                if (c != 0) return c;
                return a.value.compareTo(b.value);
            }
        });

        return records;
    }

    private static Path secureWriteFile(Path baseDir, String relativeName, List<String> lines) throws IOException {
        if (relativeName == null || relativeName.isEmpty() || relativeName.length() > 255) {
            throw new IllegalArgumentException("Invalid file name.");
        }
        Path baseReal = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path target = baseReal.resolve(relativeName).normalize();
        if (!target.startsWith(baseReal)) {
            throw new SecurityException("Write path escapes base directory.");
        }
        if (Files.exists(target, LinkOption.NOFOLLOW_LINKS)) {
            throw new FileAlreadyExistsException("File already exists: " + target);
        }
        Path parent = target.getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }
        try (BufferedWriter bw = Files.newBufferedWriter(target, StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
            for (int i = 0; i < lines.size(); i++) {
                bw.write(lines.get(i));
                bw.newLine();
            }
        }
        return target;
    }

    private static void printResult(String title, List<Pair> records) {
        System.out.println(title);
        for (Pair p : records) {
            System.out.println("  " + p);
        }
    }

    public static void main(String[] args) {
        try {
            Path base = Files.createTempDirectory("task93_kv_").toRealPath(LinkOption.NOFOLLOW_LINKS);

            // Test 1: Normal case
            secureWriteFile(base, "test1.txt", Arrays.asList("b=2", "a=1", "c=3"));
            List<Pair> r1 = readAndSortKeyValueFile(base, "test1.txt");
            printResult("Test1 sorted:", r1);

            // Test 2: Whitespace, comments, malformed lines
            secureWriteFile(base, "test2.txt", Arrays.asList("   # comment", " b = x ", "a= y", "invalidline", "=novalue", "keyonly="));
            List<Pair> r2 = readAndSortKeyValueFile(base, "test2.txt");
            printResult("Test2 sorted:", r2);

            // Test 3: Duplicate keys and varying values
            secureWriteFile(base, "test3.txt", Arrays.asList("dup=2", "dup=1", "alpha=zz", "alpha=aa"));
            List<Pair> r3 = readAndSortKeyValueFile(base, "test3.txt");
            printResult("Test3 sorted:", r3);

            // Test 4: Path traversal attempt
            try {
                readAndSortKeyValueFile(base, "../outside.txt");
                System.out.println("Test4: Unexpected success");
            } catch (Exception ex) {
                System.out.println("Test4 caught expected error: " + ex.getMessage());
            }

            // Test 5: Absolute path attempt
            Path bogus = base.resolve("test5.txt").normalize();
            secureWriteFile(base, "test5.txt", Arrays.asList("z=last", "m=mid", "a=first"));
            try {
                readAndSortKeyValueFile(base, bogus.toAbsolutePath().toString());
                System.out.println("Test5: Unexpected success");
            } catch (Exception ex) {
                System.out.println("Test5 caught expected error: " + ex.getMessage());
            }

        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }
}