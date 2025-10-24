import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.BasicFileAttributes;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.EnumSet;
import java.util.regex.Pattern;

public class Task65 {

    private static final long MAX_FILE_SIZE = 2_000_000L; // 2 MB
    private static final Pattern FILENAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");

    public static String readFileSecure(String baseDir, String userFilename) {
        if (baseDir == null || userFilename == null) {
            return "ERROR: Base directory or filename is null.";
        }

        if (!FILENAME_PATTERN.matcher(userFilename).matches()) {
            return "ERROR: Invalid filename format. Allowed: letters, digits, . _ - (1-255 chars)";
        }

        try {
            Path base = Paths.get(baseDir).toAbsolutePath().normalize();
            if (!Files.exists(base)) {
                return "ERROR: Base directory does not exist.";
            }
            if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Base path is not a directory.";
            }
            if (Files.isSymbolicLink(base)) {
                return "ERROR: Base directory must not be a symbolic link.";
            }

            Path file = base.resolve(userFilename).normalize();
            if (file.getParent() == null || !file.getParent().equals(base)) {
                return "ERROR: Resolved path is outside the base directory.";
            }

            // Reject symlinks and non-regular files
            if (!Files.exists(file, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: File does not exist.";
            }
            if (Files.isSymbolicLink(file)) {
                return "ERROR: Refusing to read a symbolic link.";
            }
            BasicFileAttributes attrs = Files.readAttributes(file, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                return "ERROR: Path is not a regular file.";
            }
            if (attrs.size() > MAX_FILE_SIZE) {
                return "ERROR: File too large (limit 2 MB).";
            }

            try (InputStream in = Files.newInputStream(file, new OpenOption[]{StandardOpenOption.READ})) {
                ByteArrayOutputStream out = new ByteArrayOutputStream();
                byte[] buffer = new byte[8192];
                long total = 0;
                int read;
                while ((read = in.read(buffer)) != -1) {
                    total += read;
                    if (total > MAX_FILE_SIZE) {
                        return "ERROR: File too large while reading (limit 2 MB).";
                    }
                    out.write(buffer, 0, read);
                }
                return new String(out.toByteArray(), StandardCharsets.UTF_8);
            } catch (IOException e) {
                return "ERROR: Unable to read file: " + e.getMessage();
            }
        } catch (IOException e) {
            return "ERROR: I/O error: " + e.getMessage();
        } catch (SecurityException se) {
            return "ERROR: Security manager denied access: " + se.getMessage();
        }
    }

    public static void main(String[] args) {
        if (args.length >= 2) {
            String result = readFileSecure(args[0], args[1]);
            System.out.println(result);
            return;
        }

        // Test cases when no command-line args are provided
        String base = ".";
        String[] tests = new String[] {
            "README.md",                // May exist or not
            "no_such_file.txt",        // Non-existent
            "bad/../path",             // Invalid format
            ".........................................................................................................................................................................................................................................................................................................................txt", // 257 dots + 'txt' (long)
            "."                        // Directory
        };

        for (int i = 0; i < tests.length; i++) {
            System.out.println("Test " + (i + 1) + " (" + tests[i] + "):");
            System.out.println(readFileSecure(base, tests[i]));
            System.out.println("-----");
        }
    }
}