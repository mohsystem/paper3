import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;

public class Task70 {
    // Maximum allowed file size to read (10 MB)
    private static final long MAX_SIZE = 10L * 1024L * 1024L;

    // Secure file-reading function:
    // - Resolves to absolute normalized path
    // - Rejects symbolic links
    // - Ensures it's a regular readable file
    // - Enforces a maximum size
    // - Streams the file to avoid TOCTOU size races
    public static String readFile(String pathStr) {
        if (pathStr == null) return null;
        try {
            Path in = Paths.get(pathStr);
            Path norm = in.toAbsolutePath().normalize();

            // Existence check without following symlinks
            if (!Files.exists(norm, LinkOption.NOFOLLOW_LINKS)) {
                return null;
            }

            // Reject symbolic links
            if (Files.isSymbolicLink(norm)) {
                return null;
            }

            // Must be a regular file and readable
            if (!Files.isRegularFile(norm, LinkOption.NOFOLLOW_LINKS) || !Files.isReadable(norm)) {
                return null;
            }

            // Initial size check
            long size = Files.size(norm);
            if (size > MAX_SIZE) {
                return null;
            }

            // Stream read with size enforcement to avoid TOCTOU size race
            try (InputStream is = Files.newInputStream(norm)) {
                ByteArrayOutputStream baos = new ByteArrayOutputStream((int) Math.min(size, 8192));
                byte[] buf = new byte[8192];
                long total = 0;
                int r;
                while ((r = is.read(buf)) != -1) {
                    total += r;
                    if (total > MAX_SIZE) {
                        return null;
                    }
                    baos.write(buf, 0, r);
                }
                return baos.toString(StandardCharsets.UTF_8);
            }
        } catch (IOException e) {
            return null;
        }
    }

    private static Path makeTempWithContent(String content) throws IOException {
        Path p = Files.createTempFile("task70_", ".txt");
        if (content != null) {
            Files.write(p, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.TRUNCATE_EXISTING);
        }
        return p;
    }

    public static void main(String[] args) throws Exception {
        if (args != null && args.length > 0) {
            for (String arg : args) {
                String out = readFile(arg);
                if (out != null) {
                    System.out.println(out);
                } else {
                    System.out.println("ERROR");
                }
            }
        } else {
            // 5 test cases
            List<Path> toDelete = new ArrayList<>();
            try {
                Path t1 = makeTempWithContent("Hello from test 1\n");
                Path t2 = makeTempWithContent("Line1\nLine2\nLine3\n");
                Path t3 = makeTempWithContent(""); // empty
                Path t4 = makeTempWithContent("Unicode: αβγ, emoji: 🙂\n");
                // Non-existent path for negative test
                String t5 = Paths.get("this_path_should_not_exist_12345.txt").toString();

                toDelete.add(t1);
                toDelete.add(t2);
                toDelete.add(t3);
                toDelete.add(t4);

                String[] tests = new String[] {
                    t1.toString(), t2.toString(), t3.toString(), t4.toString(), t5
                };

                for (int i = 0; i < tests.length; i++) {
                    String result = readFile(tests[i]);
                    System.out.println("Test " + (i + 1) + ":");
                    if (result != null) {
                        System.out.print(result);
                        if (!result.endsWith("\n")) System.out.println();
                    } else {
                        System.out.println("ERROR");
                    }
                }
            } finally {
                for (Path p : toDelete) {
                    try { Files.deleteIfExists(p); } catch (Exception ignored) {}
                }
            }
        }
    }
}