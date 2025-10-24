import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.AbstractMap.SimpleEntry;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class Task94 {

    private static final int MAX_LINE_LENGTH = 8192;
    private static final int MAX_REL_PATH_LENGTH = 4096;

    public static List<SimpleEntry<String, String>> readAndSortKeyValueFile(Path baseDir, String relativePath, char delimiter) throws IOException {
        if (baseDir == null || relativePath == null) {
            throw new IllegalArgumentException("Base directory and relative path must not be null.");
        }
        if (!Files.exists(baseDir, LinkOption.NOFOLLOW_LINKS) || !Files.isDirectory(baseDir, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Base directory does not exist or is not a directory.");
        }
        // Ensure base is not a symlink
        if (Files.isSymbolicLink(baseDir)) {
            throw new IOException("Base directory must not be a symbolic link.");
        }
        if (relativePath.length() == 0 || relativePath.length() > MAX_REL_PATH_LENGTH) {
            throw new IOException("Invalid relative path length.");
        }
        if (relativePath.indexOf('\0') >= 0) {
            throw new IOException("Invalid character in path.");
        }
        if (delimiter == '\n' || delimiter == '\r') {
            throw new IOException("Invalid delimiter.");
        }

        Path safePath = resolveSafePath(baseDir, relativePath);
        if (!Files.exists(safePath, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Target file does not exist.");
        }
        if (Files.isDirectory(safePath, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Target is a directory, expected a regular file.");
        }
        if (Files.isSymbolicLink(safePath)) {
            throw new IOException("Target file must not be a symbolic link.");
        }

        List<SimpleEntry<String, String>> entries = new ArrayList<>();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(Files.newInputStream(safePath, StandardOpenOption.READ), StandardCharsets.UTF_8))) {
            String line;
            boolean firstLine = true;
            while ((line = br.readLine()) != null) {
                if (line.length() > MAX_LINE_LENGTH) {
                    throw new IOException("Line too long.");
                }
                if (firstLine) {
                    // Strip BOM if present
                    if (!line.isEmpty() && line.charAt(0) == '\uFEFF') {
                        line = line.substring(1);
                    }
                    firstLine = false;
                }
                String trimmed = line.trim();
                if (trimmed.isEmpty() || trimmed.startsWith("#")) {
                    continue;
                }
                int idx = trimmed.indexOf(delimiter);
                if (idx < 0) {
                    // skip lines without delimiter
                    continue;
                }
                String key = trimmed.substring(0, idx).trim();
                String value = trimmed.substring(idx + 1).trim();
                if (key.isEmpty()) {
                    // skip invalid empty key
                    continue;
                }
                entries.add(new SimpleEntry<>(key, value));
            }
        }

        Comparator<SimpleEntry<String, String>> comp = Comparator
                .comparing(SimpleEntry<String, String>::getKey)
                .thenComparing(SimpleEntry<String, String>::getValue);
        Collections.sort(entries, comp);
        return entries;
    }

    private static Path resolveSafePath(Path baseDir, String relativePath) throws IOException {
        Path baseReal = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path rel = Paths.get(relativePath);
        if (rel.isAbsolute()) {
            throw new IOException("Relative path must not be absolute.");
        }
        // Reject any ".." components
        for (Path part : rel) {
            if (part.toString().equals("..")) {
                throw new IOException("Path traversal detected.");
            }
        }
        Path current = baseReal;
        for (Path part : rel.normalize()) {
            current = current.resolve(part);
            if (Files.isSymbolicLink(current)) {
                throw new IOException("Symlink encountered in path.");
            }
        }
        // Ensure final path is within base
        Path normalized = current.normalize();
        if (!normalized.startsWith(baseReal)) {
            throw new IOException("Resolved path escapes base directory.");
        }
        return normalized;
    }

    private static void writeFile(Path base, String relative, String content) {
        try {
            Path p = resolveSafePath(base, relative);
            if (Files.exists(p)) {
                throw new IOException("Test file already exists: " + p);
            }
            Files.createDirectories(p.getParent());
            Files.write(p, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private static void printResult(String title, List<SimpleEntry<String, String>> entries) {
        System.out.println(title);
        for (SimpleEntry<String, String> e : entries) {
            System.out.println(e.getKey() + "=" + e.getValue());
        }
        System.out.println("---");
    }

    public static void main(String[] args) {
        try {
            Path base = Files.createTempDirectory("kv_safe_base");
            // Test files
            writeFile(base, "t1.txt", "apple=3\nbanana=1\ncherry=2\n");
            writeFile(base, "t2.txt", "# comment\norange = 10\n banana = 5\nbanana = 2\n");
            writeFile(base, "dir/sub/t3.txt", "z=last\n a=first\nm=mid\n");
            writeFile(base, "t4.txt", "x=a=b=c\nn=:emptyBefore\n:badline\n");
            writeFile(base, "t5.txt", "key with spaces = value with spaces\nalpha= \n gamma=3\n");

            List<SimpleEntry<String, String>> r1 = readAndSortKeyValueFile(base, "t1.txt", '=');
            List<SimpleEntry<String, String>> r2 = readAndSortKeyValueFile(base, "t2.txt", '=');
            List<SimpleEntry<String, String>> r3 = readAndSortKeyValueFile(base, "dir/sub/t3.txt", '=');
            List<SimpleEntry<String, String>> r4 = readAndSortKeyValueFile(base, "t4.txt", '=');
            List<SimpleEntry<String, String>> r5 = readAndSortKeyValueFile(base, "t5.txt", '=');

            printResult("Test 1:", r1);
            printResult("Test 2:", r2);
            printResult("Test 3:", r3);
            printResult("Test 4:", r4);
            printResult("Test 5:", r5);

        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}