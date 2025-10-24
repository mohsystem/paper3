import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Task36 {

    // Reads a file at filePath, prints its contents to the console, and returns the contents as a String.
    public static String readFileAndPrint(String filePath) {
        final long MAX_BYTES = 10L * 1024L * 1024L; // 10 MB limit
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: file path is null or empty.");
            return "";
        }

        Path p;
        try {
            p = Paths.get(filePath);
        } catch (InvalidPathException e) {
            System.err.println("Error: invalid file path.");
            return "";
        }

        try {
            if (!Files.exists(p, LinkOption.NOFOLLOW_LINKS)) {
                System.err.println("Error: file does not exist.");
                return "";
            }
            if (!Files.isRegularFile(p, LinkOption.NOFOLLOW_LINKS)) {
                System.err.println("Error: not a regular file.");
                return "";
            }
            if (!Files.isReadable(p)) {
                System.err.println("Error: file is not readable.");
                return "";
            }
        } catch (SecurityException se) {
            System.err.println("Error: access denied to the file.");
            return "";
        }

        StringBuilder sb = new StringBuilder(8192);
        long total = 0L;
        try (InputStream in = Files.newInputStream(p, StandardOpenOption.READ)) {
            byte[] buf = new byte[8192];
            int r;
            while ((r = in.read(buf)) != -1) {
                total += r;
                if (total > MAX_BYTES) {
                    System.err.println("Error: file exceeds maximum allowed size of 10 MB.");
                    return "";
                }
                sb.append(new String(buf, 0, r, StandardCharsets.UTF_8));
            }
        } catch (IOException ioe) {
            System.err.println("Error: I/O error while reading the file.");
            return "";
        }

        String out = sb.toString();
        System.out.print(out);
        return out;
    }

    public static void main(String[] args) throws Exception {
        // Create 4 temporary files for testing and 1 non-existent path
        Path t1 = Files.createTempFile("task36_", ".txt");
        Path t2 = Files.createTempFile("task36_", ".txt");
        Path t3 = Files.createTempFile("task36_", ".txt");
        Path t4 = Files.createTempFile("task36_", ".txt");

        Files.write(t1, "Hello, Task36!\n".getBytes(StandardCharsets.UTF_8));
        Files.write(t2, new byte[0]); // empty file
        Files.write(t3, "こんにちは世界\nEmoji: 😀\n".getBytes(StandardCharsets.UTF_8));
        Files.write(t4, "Line1\nLine2\nLine3\n".getBytes(StandardCharsets.UTF_8));

        String nonExistent = t1.getParent().resolve("does_not_exist_" + System.nanoTime() + ".txt").toString();

        // Run 5 test cases
        readFileAndPrint(t1.toString());
        readFileAndPrint(t2.toString());
        readFileAndPrint(t3.toString());
        readFileAndPrint(t4.toString());
        readFileAndPrint(nonExistent);
    }
}