import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class Task39 {

    public static String displayFileSecure(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Invalid filename.";
        }
        if (filename.indexOf('\0') >= 0) {
            return "Error: Invalid character in filename.";
        }

        try {
            Path path = Paths.get(filename).toAbsolutePath().normalize();
            if (!Files.exists(path)) {
                return "Error: File does not exist.";
            }
            if (!Files.isRegularFile(path, LinkOption.NOFOLLOW_LINKS)) {
                return "Error: Not a regular file.";
            }
            if (!Files.isReadable(path)) {
                return "Error: File is not readable.";
            }

            boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
            List<String> cmd = new ArrayList<>();
            if (isWindows) {
                cmd.add("more");
            } else {
                cmd.add("cat");
            }
            cmd.add(path.toString());

            ProcessBuilder pb = new ProcessBuilder(cmd);
            pb.redirectErrorStream(true);
            Process proc = pb.start();

            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            try (InputStream is = proc.getInputStream()) {
                byte[] buf = new byte[8192];
                int r;
                while ((r = is.read(buf)) != -1) {
                    baos.write(buf, 0, r);
                }
            }

            boolean finished = proc.waitFor(5, TimeUnit.SECONDS);
            if (!finished) {
                proc.destroyForcibly();
                return "Error: Command timed out.";
            }

            int exit = proc.exitValue();
            String output = baos.toString(StandardCharsets.UTF_8);
            if (exit != 0) {
                return "Error: Command failed with exit code " + exit + ". Output: " + output;
            }
            return output;
        } catch (IOException | InterruptedException e) {
            Thread.currentThread().interrupt();
            return "Error: " + e.getMessage();
        }
    }

    public static void main(String[] args) throws Exception {
        // Prepare 5 test cases
        Path tempDir = Files.createTempDirectory("task39_");
        Path f1 = tempDir.resolve("file1.txt");
        Path f2 = tempDir.resolve("file with spaces.txt");
        Path f3 = tempDir.resolve("empty.txt");
        Path invalid = tempDir.resolve("no_such_file.txt");
        Path dir = tempDir.resolve("subdir");

        Files.writeString(f1, "Hello from file1\nLine 2\n", StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
        Files.writeString(f2, "Content with spaces in filename\n", StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
        Files.writeString(f3, "", StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
        Files.createDirectories(dir);

        System.out.println("Test 1 (regular file):");
        System.out.println(displayFileSecure(f1.toString()));

        System.out.println("Test 2 (file with spaces):");
        System.out.println(displayFileSecure(f2.toString()));

        System.out.println("Test 3 (empty file):");
        System.out.println(displayFileSecure(f3.toString()));

        System.out.println("Test 4 (invalid path):");
        System.out.println(displayFileSecure(invalid.toString()));

        System.out.println("Test 5 (directory path):");
        System.out.println(displayFileSecure(dir.toString()));
    }
}