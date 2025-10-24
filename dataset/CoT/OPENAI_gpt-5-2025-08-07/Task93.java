import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.TreeMap;

public class Task93 {

    // Step 1: Implement core functionality to read and sort key-value records from a file.
    // Step 2: Apply basic validations and safe file operations.
    // Step 3: Ensure robust parsing and error handling.
    // Step 4: Review: avoid unsafe operations, handle edge cases.
    // Step 5: Finalize secure output.

    public static LinkedHashMap<String, String> readAndSortKeyValues(String filePath) throws IOException {
        if (filePath == null) {
            throw new IllegalArgumentException("filePath cannot be null");
        }
        final long MAX_FILE_SIZE = 10L * 1024L * 1024L; // 10 MB limit as a precaution
        final int MAX_KEY_LEN = 4096;
        final int MAX_VALUE_LEN = 2_000_000;

        Path path;
        try {
            path = Paths.get(filePath).normalize();
        } catch (InvalidPathException ipe) {
            throw new IOException("Invalid file path", ipe);
        }

        if (!Files.exists(path)) {
            throw new NoSuchFileException("File does not exist: " + filePath);
        }
        if (Files.isDirectory(path)) {
            throw new IOException("Path is a directory: " + filePath);
        }
        try {
            long size = Files.size(path);
            if (size > MAX_FILE_SIZE) {
                throw new IOException("File too large: " + size + " bytes");
            }
        } catch (IOException e) {
            // If size cannot be determined, proceed but still read safely line by line
        }

        TreeMap<String, String> sorted = new TreeMap<>();
        try (BufferedReader br = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            long totalValueBytes = 0;
            while ((line = br.readLine()) != null) {
                String trimmed = line.trim();
                if (trimmed.isEmpty() || trimmed.startsWith("#") || trimmed.startsWith("//")) continue;

                int idxEq = trimmed.indexOf('=');
                int idxCol = trimmed.indexOf(':');
                int idx;
                if (idxEq == -1 && idxCol == -1) continue;
                else if (idxEq == -1) idx = idxCol;
                else if (idxCol == -1) idx = idxEq;
                else idx = Math.min(idxEq, idxCol);

                String key = trimmed.substring(0, idx).trim();
                String value = trimmed.substring(idx + 1).trim();
                if (key.isEmpty()) continue;

                if (key.length() > MAX_KEY_LEN) {
                    throw new IOException("Key too long");
                }
                if (value.length() > MAX_VALUE_LEN) {
                    throw new IOException("Value too long");
                }
                totalValueBytes += value.length();
                if (totalValueBytes > MAX_FILE_SIZE) {
                    throw new IOException("Accumulated values exceed safe threshold");
                }
                sorted.put(key, value); // last one wins on duplicate keys
            }
        }

        LinkedHashMap<String, String> result = new LinkedHashMap<>();
        for (Map.Entry<String, String> e : sorted.entrySet()) {
            result.put(e.getKey(), e.getValue());
        }
        return result;
    }

    private static void writeFile(Path p, String content) {
        try {
            Files.createDirectories(p.getParent() == null ? Paths.get(".") : p.getParent());
            try (BufferedWriter bw = Files.newBufferedWriter(p, StandardCharsets.UTF_8, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
                bw.write(content);
            }
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private static void runTest(String title, String fileName, String content) throws IOException {
        Path p = Paths.get(fileName).normalize();
        writeFile(p, content);
        LinkedHashMap<String, String> out = readAndSortKeyValues(p.toString());
        System.out.println("=== " + title + " (" + p + ") ===");
        for (Map.Entry<String, String> e : out.entrySet()) {
            System.out.println(e.getKey() + "=" + e.getValue());
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        runTest("Test 1 - Basic", "test_kv_1.txt",
                "banana=yellow\napple=red\ncarrot=orange\n");
        runTest("Test 2 - Unsorted with spaces", "test_kv_2.txt",
                "  zeta = 26 \nalpha=1\nbeta = 2\n  gamma= 3\n");
        runTest("Test 3 - Duplicates", "test_kv_3.txt",
                "a=1\na=2\nb=3\nA=upper\n");
        runTest("Test 4 - Mixed separators and comments", "test_kv_4.txt",
                "# Comment line\n// Another comment\nhost: localhost\nport=8080\n user : admin \npassword = secret\n");
        runTest("Test 5 - Special characters", "test_kv_5.txt",
                "path=/usr/local/bin\nkey.with.dots=value.with.dots\n spaced key = spaced value \nemoji=🙂\ncolon:used\n");
    }
}