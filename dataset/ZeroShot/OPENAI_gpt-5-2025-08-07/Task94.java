import java.io.BufferedReader;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class Task94 {

    private static final int MAX_LINE_LEN = 1_048_576;

    private static boolean isCommentOrEmpty(String trimmed) {
        return trimmed.isEmpty() || trimmed.startsWith("#") || trimmed.startsWith(";"); 
    }

    public static List<String> readAndSortKeyValueFile(String filePath) throws IOException {
        if (filePath == null || filePath.isEmpty()) {
            throw new IllegalArgumentException("filePath must not be null or empty");
        }
        Path path = Paths.get(filePath);
        List<String[]> pairs = new ArrayList<>();
        try (BufferedReader br = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                if (line.length() > MAX_LINE_LEN) {
                    // Skip overly long lines to avoid memory issues
                    continue;
                }
                // Handle Windows-style newlines safely (readLine already strips them).
                String trimmed = line.trim();
                if (isCommentOrEmpty(trimmed)) {
                    continue;
                }
                int idx = trimmed.indexOf('=');
                if (idx <= 0) {
                    // no '=' or empty key
                    continue;
                }
                String key = trimmed.substring(0, idx).trim();
                String value = trimmed.substring(idx + 1).trim();
                if (key.isEmpty()) {
                    continue;
                }
                pairs.add(new String[]{key, value});
            }
        }
        // Sort by key, then by value
        pairs.sort(Comparator.<String[], String>comparing(a -> a[0]).thenComparing(a -> a[1]));
        List<String> out = new ArrayList<>(pairs.size());
        for (String[] kv : pairs) {
            out.add(kv[0] + "=" + kv[1]);
        }
        return out;
    }

    // Helper for tests to create temp files with content
    private static Path createTempFileWithContent(String content) {
        try {
            Path tmp = Files.createTempFile("task94_test_", ".txt");
            Files.write(tmp, content.getBytes(StandardCharsets.UTF_8));
            return tmp;
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private static void printResult(String title, List<String> result) {
        System.out.println(title);
        for (String s : result) {
            System.out.println(s);
        }
        System.out.println("----");
    }

    public static void main(String[] args) throws Exception {
        String c1 = ""
                + "# Comment line\n"
                + "a=1\n"
                + "b= 2\n"
                + " c =3 \n"
                + "d=4\n"
                + "invalidline\n"
                + "=novalue\n"
                + "e=\n"
                + ";comment\n"
                + "f = value with spaces  \n";
        String c2 = ""
                + "z = last\n"
                + "a = first\n"
                + "m = middle\n"
                + "a = duplicate\n"
                + "x=42\n"
                + "y=100\n";
        String c3 = ""
                + " key = value=with=equals\n"
                + " spaced key = spaced value \n"
                + "# comment\n"
                + "emptykey= \n"
                + " = bad\n"
                + "onlykey=\n";
        String c4 = ""
                + "café=au lait\n"
                + "naïve=façade\n"
                + "日本=語\n"
                + "ключ=значение\n";
        String c5 = ""
                + "delta=4\r\n"
                + "alpha=1\r\n"
                + "charlie=3\r\n"
                + "bravo=2\r\n"
                + "alpha=0\r\n";

        Path p1 = createTempFileWithContent(c1);
        Path p2 = createTempFileWithContent(c2);
        Path p3 = createTempFileWithContent(c3);
        Path p4 = createTempFileWithContent(c4);
        Path p5 = createTempFileWithContent(c5);

        try {
            printResult("Test 1", readAndSortKeyValueFile(p1.toString()));
            printResult("Test 2", readAndSortKeyValueFile(p2.toString()));
            printResult("Test 3", readAndSortKeyValueFile(p3.toString()));
            printResult("Test 4", readAndSortKeyValueFile(p4.toString()));
            printResult("Test 5", readAndSortKeyValueFile(p5.toString()));
        } finally {
            try { Files.deleteIfExists(p1); } catch (IOException ignored) {}
            try { Files.deleteIfExists(p2); } catch (IOException ignored) {}
            try { Files.deleteIfExists(p3); } catch (IOException ignored) {}
            try { Files.deleteIfExists(p4); } catch (IOException ignored) {}
            try { Files.deleteIfExists(p5); } catch (IOException ignored) {}
        }
    }
}