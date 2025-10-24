import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

public class Task99 {
    public static List<String> matchLines(String filePath) throws IOException {
        List<String> result = new ArrayList<>();
        Pattern pattern = Pattern.compile("^\\d+.*\\.$");
        try (BufferedReader br = Files.newBufferedReader(Paths.get(filePath), StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                // br.readLine() removes line terminators; match directly
                if (pattern.matcher(line).matches()) {
                    result.add(line);
                }
            }
        }
        return result;
    }

    private static void writeFileLines(String path, List<String> lines) throws IOException {
        Files.write(Paths.get(path), lines, StandardCharsets.UTF_8);
    }

    private static void writeFileRaw(String path, String content) throws IOException {
        Files.write(Paths.get(path), content.getBytes(StandardCharsets.UTF_8));
    }

    public static void main(String[] args) throws Exception {
        // Test case 1
        String f1 = "test1.txt";
        writeFileLines(f1, Arrays.asList(
                "123 Start here.",
                "Not starting with number.",
                "42 Another line.",
                "7 ends with no dot",
                "0."
        ));
        System.out.println("Test 1 matches: " + matchLines(f1));

        // Test case 2
        String f2 = "test2.txt";
        writeFileLines(f2, Arrays.asList(
                "10 Leading digits not ending with dot",
                "20 ends with dot.",
                ". starts with dot.",
                "99Trailing spaces."
        ));
        System.out.println("Test 2 matches: " + matchLines(f2));

        // Test case 3
        String f3 = "test3.txt";
        writeFileLines(f3, Arrays.asList(
                "No numbers at start.",
                "3.14159.",
                "31415",
                "2718."
        ));
        System.out.println("Test 3 matches: " + matchLines(f3));

        // Test case 4
        String f4 = "test4.txt";
        writeFileLines(f4, Arrays.asList(
                "A line.",
                "123.",
                "abc123.",
                "456 trailing spaces .",
                "789!."
        ));
        System.out.println("Test 4 matches: " + matchLines(f4));

        // Test case 5 (CRLF endings)
        String f5 = "test5.txt";
        String content5 = "100 First line.\r\n" +
                          "no number.\r\n" +
                          "200 End with dot.\r\n" +
                          "300 no dot\r\n" +
                          "400.\r\n";
        writeFileRaw(f5, content5);
        System.out.println("Test 5 matches: " + matchLines(f5));
    }
}