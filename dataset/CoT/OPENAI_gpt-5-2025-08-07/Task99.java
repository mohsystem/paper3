import java.io.BufferedWriter;
import java.io.IOException;
import java.io.BufferedReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.regex.*;

public class Task99 {

    // Core function: reads a file and returns lines that start with numbers and end with a dot.
    public static List<String> matchLinesStartingWithNumbersEndingWithDot(Path filePath) throws IOException {
        List<String> matches = new ArrayList<>();
        if (filePath == null) {
            return matches;
        }
        if (!Files.exists(filePath) || !Files.isRegularFile(filePath) || !Files.isReadable(filePath)) {
            return matches;
        }

        Pattern pattern = Pattern.compile("^\\d+.*\\.$");
        try (BufferedReader reader = Files.newBufferedReader(filePath, StandardCharsets.UTF_8)) {
            String line;
            while ((line = reader.readLine()) != null) {
                // Normalize possible CR from Windows line endings
                if (!line.isEmpty() && line.charAt(line.length() - 1) == '\r') {
                    line = line.substring(0, line.length() - 1);
                }
                if (pattern.matcher(line).matches()) {
                    matches.add(line);
                }
            }
        }
        return matches;
    }

    private static Path createTempFileWithContent(String prefix, List<String> lines) throws IOException {
        Path p = Files.createTempFile(prefix, ".txt");
        p.toFile().deleteOnExit();
        try (BufferedWriter w = Files.newBufferedWriter(p, StandardCharsets.UTF_8, StandardOpenOption.TRUNCATE_EXISTING)) {
            for (String s : lines) {
                w.write(s);
                w.newLine();
            }
        }
        return p;
    }

    private static void printResult(String title, List<String> result) {
        System.out.println(title);
        for (String s : result) {
            System.out.println(s);
        }
        System.out.println("----");
    }

    public static void main(String[] args) throws Exception {
        // Test case 1: mixed lines
        Path f1 = createTempFileWithContent("task99_test1_", Arrays.asList(
                "123 This line starts with numbers and ends with a dot.",
                "Not starting with number.",
                "42.",
                "7 ends without dot",
                "007 Bond."
        ));
        printResult("Test 1:", matchLinesStartingWithNumbersEndingWithDot(f1));

        // Test case 2: only matching lines
        Path f2 = createTempFileWithContent("task99_test2_", Arrays.asList(
                "1.",
                "22.",
                "333.",
                "4444."
        ));
        printResult("Test 2:", matchLinesStartingWithNumbersEndingWithDot(f2));

        // Test case 3: no matching lines
        Path f3 = createTempFileWithContent("task99_test3_", Arrays.asList(
                "hello.",
                ".",
                " no number.",
                "123 but no dot at end",
                ""
        ));
        printResult("Test 3:", matchLinesStartingWithNumbersEndingWithDot(f3));

        // Test case 4: edge cases and punctuation
        Path f4 = createTempFileWithContent("task99_test4_", Arrays.asList(
                "0.",
                "9 Ends without dot",
                "123abc.",
                "999!.",
                "001 leading zeros."
        ));
        printResult("Test 4:", matchLinesStartingWithNumbersEndingWithDot(f4));

        // Test case 5: multiple dots and windows-style endings simulation
        Path f5 = createTempFileWithContent("task99_test5_", Arrays.asList(
                "123.456.",
                "10. end.",
                "5... ... ...",
                "8.",
                "12.3" // no final dot
        ));
        printResult("Test 5:", matchLinesStartingWithNumbersEndingWithDot(f5));
    }
}