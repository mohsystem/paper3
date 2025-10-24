import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.*;

public class Task100 {
    private static final long MAX_BYTES = 5L * 1024L * 1024L; // 5 MB
    private static final int MAX_PATTERN_LEN = 10000;

    public static List<String> searchInFile(String regex, String filePath) throws IOException {
        if (regex == null || filePath == null) {
            throw new IllegalArgumentException("Pattern and file path must not be null.");
        }
        if (regex.length() > MAX_PATTERN_LEN) {
            throw new IllegalArgumentException("Pattern too long.");
        }
        Path path = Paths.get(filePath);
        if (Files.isSymbolicLink(path)) {
            throw new SecurityException("Refusing to follow symbolic links.");
        }
        BasicFileAttributes attrs = Files.readAttributes(path, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!attrs.isRegularFile()) {
            throw new IOException("Not a regular file.");
        }
        if (attrs.size() > MAX_BYTES) {
            throw new IOException("File too large.");
        }
        String content = Files.readString(path, StandardCharsets.UTF_8);
        List<String> matches = new ArrayList<>();
        try {
            Pattern pattern = Pattern.compile(regex);
            Matcher matcher = pattern.matcher(content);
            while (matcher.find()) {
                matches.add(matcher.group());
            }
        } catch (PatternSyntaxException e) {
            throw new IllegalArgumentException("Invalid regex pattern: " + e.getDescription(), e);
        }
        return matches;
    }

    private static Path createTempFileWithContent(String content) throws IOException {
        Path temp = Files.createTempFile("task100_", ".txt");
        Files.writeString(temp, content, StandardCharsets.UTF_8, StandardOpenOption.TRUNCATE_EXISTING);
        return temp;
    }

    private static void runTests() {
        String content = "The quick brown fox jumps over the lazy dog.\nNumbers: 12345 and 67890.\nTHE end.\n";
        String[] patterns = new String[] {
            "fox",
            "[A-Za-z]{4}",
            "[Tt][Hh][Ee]",
            "[0-9]+",
            "notfound"
        };
        Path tempFile = null;
        try {
            tempFile = createTempFileWithContent(content);
            System.out.println("Test file: " + tempFile.toString());
            for (int i = 0; i < patterns.length; i++) {
                String p = patterns[i];
                System.out.println("Test " + (i + 1) + " Pattern: " + p);
                List<String> results = searchInFile(p, tempFile.toString());
                System.out.println("Matches found: " + results.size());
                for (String m : results) {
                    System.out.println(m);
                }
                System.out.println("---");
            }
        } catch (Exception e) {
            System.err.println("Test error: " + e.getMessage());
        } finally {
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException ignored) {}
            }
        }
    }

    public static void main(String[] args) {
        if (args.length == 2) {
            try {
                List<String> matches = searchInFile(args[0], args[1]);
                System.out.println("Matches found: " + matches.size());
                for (String m : matches) {
                    System.out.println(m);
                }
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
                System.exit(1);
            }
        } else {
            runTests();
        }
    }
}