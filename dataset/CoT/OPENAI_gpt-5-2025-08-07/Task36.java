// Chain-of-Through process:
// 1) Problem understanding: Implement a secure function that reads a user-specified file and returns its content; main prints it.
// 2) Security requirements: Avoid symlink traversal, ensure it's a regular file, enforce size limits, use safe I/O with proper error handling.
// 3) Secure coding generation: Implement checks with Java NIO, no following symlinks, size cap, buffered reads, UTF-8 decoding.
// 4) Code review: Validate parameters, handle exceptions, avoid resource leaks (try-with-resources), avoid directory traversal issues by normalizing path.
// 5) Secure code output: Final code with mitigations and test cases.

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;

public class Task36 {
    private static final long MAX_FILE_SIZE = 10L * 1024L * 1024L; // 10 MB

    // Secure file reader: accepts input path as parameter and returns file content as String.
    public static String readFileSecure(String path) throws IOException {
        if (path == null) {
            throw new IllegalArgumentException("Path is null");
        }
        Path p = Paths.get(path).normalize();

        if (!Files.exists(p, LinkOption.NOFOLLOW_LINKS)) {
            throw new FileNotFoundException("File does not exist");
        }
        if (Files.isSymbolicLink(p)) {
            throw new IOException("Refusing to read symbolic link");
        }
        BasicFileAttributes attrs = Files.readAttributes(p, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!attrs.isRegularFile()) {
            throw new IOException("Not a regular file");
        }
        long size = attrs.size();
        if (size > MAX_FILE_SIZE) {
            throw new IOException("File too large (limit " + MAX_FILE_SIZE + " bytes)");
        }

        try (InputStream in = new BufferedInputStream(Files.newInputStream(p, StandardOpenOption.READ))) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream(size > 0 && size < Integer.MAX_VALUE ? (int) size : 8192);
            byte[] buf = new byte[8192];
            int r;
            while ((r = in.read(buf)) != -1) {
                baos.write(buf, 0, r);
                if (baos.size() > MAX_FILE_SIZE) {
                    throw new IOException("File grew beyond size limit while reading");
                }
            }
            return baos.toString(StandardCharsets.UTF_8);
        }
    }

    public static void main(String[] args) {
        // If a path is provided by user, read and print it.
        if (args.length > 0) {
            try {
                String content = readFileSecure(args[0]);
                System.out.print(content);
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }
        }

        // 5 test cases
        try {
            Path dir = Files.createTempDirectory("task36_tests_java");
            // Test 1: small file
            Path t1 = dir.resolve("small.txt");
            Files.writeString(t1, "Hello\nWorld\n", StandardCharsets.UTF_8, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            // Test 2: empty file
            Path t2 = dir.resolve("empty.txt");
            Files.write(t2, new byte[0]);

            // Test 3: unicode content
            Path t3 = dir.resolve("unicode.txt");
            Files.writeString(t3, "こんにちは\n🙂\n", StandardCharsets.UTF_8, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            // Test 4: non-existent file
            Path t4 = dir.resolve("missing.txt"); // do not create

            // Test 5: near-limit file (1KB)
            Path t5 = dir.resolve("limit.txt");
            byte[] arr = "A".repeat(1024).getBytes(StandardCharsets.UTF_8);
            Files.write(t5, arr, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            String[] tests = { t1.toString(), t2.toString(), t3.toString(), t4.toString(), t5.toString() };
            for (String test : tests) {
                System.out.println("\n--- Reading: " + test + " ---");
                try {
                    String c = readFileSecure(test);
                    System.out.print(c);
                } catch (Exception e) {
                    System.out.println("Error: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Test setup error: " + e.getMessage());
        }
    }
}