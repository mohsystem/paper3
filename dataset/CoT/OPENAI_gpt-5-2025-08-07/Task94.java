import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.AbstractMap.SimpleEntry;

public class Task94 {
    // Chain-of-Through process:
    // 1) Problem understanding: Read key=value records from a file, one by one, ignore invalid/comment lines, sort by key then value, return sorted records.
    // 2) Security requirements: Avoid executing external input; limit processing, trim inputs, handle errors, avoid unsafe operations, use UTF-8 decoding.
    // 3) Secure coding generation: Implement bounded processing, ignore malformed input, do not allow empty keys, avoid crashes on I/O errors.
    // 4) Code review: Ensure no external command execution, safe resource handling, sorting with deterministic comparator, comments/plain text lines ignored.
    // 5) Secure code output: Final code below with tests.

    private static final int MAX_RECORDS = 1_000_000;

    public static List<SimpleEntry<String, String>> readAndSortRecords(String filePath) throws IOException {
        List<SimpleEntry<String, String>> records = new ArrayList<>();
        try (BufferedReader br = Files.newBufferedReader(Paths.get(filePath), StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                // Normalize line endings and trim
                String trimmed = line.trim();
                if (trimmed.isEmpty()) continue;
                if (trimmed.startsWith("#")) continue; // comment line
                int eq = trimmed.indexOf('=');
                if (eq <= 0) continue; // no '=' or empty key
                String key = trimmed.substring(0, eq).trim();
                if (key.isEmpty()) continue; // ignore empty key
                String value = trimmed.substring(eq + 1).trim();
                records.add(new SimpleEntry<>(key, value));
                if (records.size() > MAX_RECORDS) {
                    throw new IOException("Too many records; possible resource exhaustion.");
                }
            }
        }
        records.sort((a, b) -> {
            int c = a.getKey().compareTo(b.getKey());
            if (c != 0) return c;
            return a.getValue().compareTo(b.getValue());
        });
        return records;
    }

    // Helper to stringify results
    public static String recordsToString(List<SimpleEntry<String, String>> recs) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < recs.size(); i++) {
            SimpleEntry<String, String> e = recs.get(i);
            sb.append(e.getKey()).append("=").append(e.getValue());
            if (i + 1 < recs.size()) sb.append(", ");
        }
        sb.append("]");
        return sb.toString();
    }

    private static Path writeTempFile(String content) throws IOException {
        Path p = Files.createTempFile("task94_", ".txt");
        Files.write(p, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.TRUNCATE_EXISTING);
        return p;
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        List<String> contents = Arrays.asList(
            // 1
            "b=2\na=1\nc=3\n",
            // 2
            "  key2 =  z \n# comment\n\nkey1= a \n  # another comment\n",
            // 3
            "k=2\nk=1\nk=3\n",
            // 4
            "noequal\n=onlyvalue\nonlykey=\n mid = val = extra \n",
            // 5
            "äpple=1\nbanana=2\nÁlpha=0\n"
        );

        List<Path> files = new ArrayList<>();
        try {
            for (String c : contents) files.add(writeTempFile(c));

            for (int i = 0; i < files.size(); i++) {
                List<SimpleEntry<String, String>> out = readAndSortRecords(files.get(i).toString());
                System.out.println("Test " + (i + 1) + " result: " + recordsToString(out));
            }
        } finally {
            // Cleanup
            for (Path p : files) {
                try { Files.deleteIfExists(p); } catch (Exception ignore) {}
            }
        }
    }
}