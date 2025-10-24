import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task100 {
    // Chain-of-Through process in code comments:
    // 1) Problem understanding: Function compiles a regex and searches a file's content for matches, returning them.
    // 2) Security requirements: Limit file read size, handle invalid regex, avoid infinite loops with zero-length matches, handle IO exceptions safely.
    // 3) Secure coding generation: Use buffered reads, size caps, safe charset, bounded matches, robust exception handling.
    // 4) Code review: Ensured resources are closed, limits enforced, exceptions handled without leaking sensitive info.
    // 5) Secure code output: Final code reflects mitigations and safe defaults.

    public static List<String> regexSearchInFile(String regex, String filePath) {
        return regexSearchInFile(regex, filePath, 1_048_576, 10_000); // 1 MiB limit, 10k matches cap
    }

    public static List<String> regexSearchInFile(String regex, String filePath, int maxBytes, int maxMatches) {
        List<String> matches = new ArrayList<>();
        if (regex == null || filePath == null) return matches;

        String content = "";
        try (InputStream in = new BufferedInputStream(Files.newInputStream(Path.of(filePath)))) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[8192];
            int remaining = Math.max(0, maxBytes);
            int read;
            while (remaining > 0 && (read = in.read(buf, 0, Math.min(buf.length, remaining))) != -1) {
                baos.write(buf, 0, read);
                remaining -= read;
            }
            content = new String(baos.toByteArray(), StandardCharsets.UTF_8);
        } catch (Exception e) {
            return matches;
        }

        Pattern p;
        try {
            p = Pattern.compile(regex, Pattern.MULTILINE);
        } catch (Exception e) {
            return matches;
        }

        try {
            Matcher m = p.matcher(content);
            int count = 0;
            int lastEnd = -1;
            while (m.find()) {
                matches.add(m.group());
                count++;
                if (count >= maxMatches) break;

                // Prevent potential infinite loop on zero-length matches
                if (m.start() == m.end()) {
                    int nextIndex = m.end() + 1;
                    if (nextIndex > content.length()) break;
                    m.region(nextIndex, content.length());
                }
                lastEnd = m.end();
            }
        } catch (Exception e) {
            // In case of unexpected matching errors, return what we have so far
        }
        return matches;
    }

    private static Path createTempFileWithContent(String prefix, String content) throws Exception {
        Path path = Files.createTempFile(prefix, ".txt");
        Files.write(path, content.getBytes(StandardCharsets.UTF_8));
        path.toFile().deleteOnExit();
        return path;
    }

    private static void runTest(String title, String pattern, String content) throws Exception {
        Path f = createTempFileWithContent("task100_", content);
        List<String> res = regexSearchInFile(pattern, f.toString());
        System.out.println("[" + title + "] pattern=" + pattern + " file=" + f);
        System.out.println("matches=" + res.size());
        for (int i = 0; i < Math.min(res.size(), 10); i++) {
            System.out.println(res.get(i));
        }
        System.out.println("---");
        try { Files.deleteIfExists(f); } catch (Exception ignored) {}
    }

    public static void main(String[] args) throws Exception {
        if (args != null && args.length >= 2) {
            String pattern = args[0];
            String file = args[1];
            List<String> matches = regexSearchInFile(pattern, file);
            for (String s : matches) {
                System.out.println(s);
            }
            return;
        }

        // 5 test cases
        runTest("T1 simple literal", "foo", "foo bar foo\nbaz");
        runTest("T2 3 letters", "[A-Za-z]{3}", "abc XYZ abcdef");
        runTest("T3 digits", "[0-9]+", "Order 123: 456 items.");
        runTest("T4 end anchor", "end$", "line with end");
        runTest("T5 optional u", "colou?r", "color colour colr");
    }
}