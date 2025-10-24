import java.io.BufferedWriter;
import java.io.IOException;
import java.io.BufferedReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List; 
import java.util.regex.Pattern;

public class Task99 {

    private static final Pattern PATTERN = Pattern.compile("^[0-9]+.*\\.$");

    public static List<String> matchLinesStartNumEndDot(String filePath) throws IOException {
        if (filePath == null) {
            throw new IllegalArgumentException("filePath must not be null");
        }
        Path path = Paths.get(filePath);
        List<String> matches = new ArrayList<>();
        try (BufferedReader reader = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            while ((line = reader.readLine()) != null) {
                // line is already without newline
                if (PATTERN.matcher(line).matches()) {
                    matches.add(line);
                }
            }
        }
        return matches;
    }

    private static Path createTempFileWithLines(List<String> lines) throws IOException {
        Path tmp = Files.createTempFile("task99_", ".txt");
        try (BufferedWriter w = Files.newBufferedWriter(tmp, StandardCharsets.UTF_8)) {
            for (String s : lines) {
                w.write(s);
                w.newLine();
            }
        }
        tmp.toFile().deleteOnExit();
        return tmp;
    }

    private static void printTest(String title, List<String> out) {
        System.out.println(title);
        for (String s : out) {
            System.out.println(s);
        }
        System.out.println("---");
    }

    public static void main(String[] args) throws Exception {
        // Test case 1
        Path f1 = createTempFileWithLines(List.of(
            "123.",
            "1abc.",
            "abc.",
            "123",
            " 123.",
            "456.."
        ));
        printTest("Test 1", matchLinesStartNumEndDot(f1.toString()));

        // Test case 2: empty file
        Path f2 = createTempFileWithLines(List.of());
        printTest("Test 2", matchLinesStartNumEndDot(f2.toString()));

        // Test case 3: whitespace and numbers
        Path f3 = createTempFileWithLines(List.of(
            "\t789.",
            "000.",
            "000. ",
            "999.x",
            "42."
        ));
        printTest("Test 3", matchLinesStartNumEndDot(f3.toString()));

        // Test case 4: mixed punctuation
        Path f4 = createTempFileWithLines(List.of(
            "7!@#$.",
            "5text.",
            "0042..",
            "nope.",
            "9\n"
        ));
        printTest("Test 4", matchLinesStartNumEndDot(f4.toString()));

        // Test case 5: long lines and edge cases
        StringBuilder longLine = new StringBuilder();
        for (int i = 0; i < 1000; i++) longLine.append('x');
        Path f5 = createTempFileWithLines(List.of(
            "1" + longLine + ".",
            longLine.toString() + ".",
            "2.",
            "3. ",
            "4.."
        ));
        printTest("Test 5", matchLinesStartNumEndDot(f5.toString()));
    }
}