import java.io.IOException;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class Task190 {

    /**
     * Transposes the content of a text file.
     *
     * @param filePath The path to the file.
     * @return A string with the transposed content, or an error message.
     */
    public static String transposeFile(String filePath) {
        Path path = Paths.get(filePath);
        if (!Files.exists(path)) {
            return "Error: File not found.";
        }

        try {
            List<String> lines = Files.readAllLines(path, StandardCharsets.UTF_8);
            if (lines.isEmpty()) {
                return "";
            }

            List<String[]> grid = new ArrayList<>();
            for (String line : lines) {
                String[] words = line.trim().split("\\s+");
                // Handle case where a line might be empty or just whitespace
                if (words.length == 1 && words[0].isEmpty()) {
                    continue;
                }
                grid.add(words);
            }

            if (grid.isEmpty()) {
                return "";
            }

            int rows = grid.size();
            int cols = grid.get(0).length;

            StringBuilder transposedContent = new StringBuilder();
            for (int j = 0; j < cols; j++) {
                for (int i = 0; i < rows; i++) {
                    // Assuming all rows have the same number of columns as per prompt.
                    transposedContent.append(grid.get(i)[j]);
                    if (i < rows - 1) {
                        transposedContent.append(" ");
                    }
                }
                if (j < cols - 1) {
                    transposedContent.append("\n");
                }
            }

            return transposedContent.toString();

        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        }
    }

    private static void createTestFile(String fileName, String content) throws IOException {
        try (PrintWriter writer = new PrintWriter(fileName, StandardCharsets.UTF_8.name())) {
            writer.print(content);
        }
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {"name age\nalice 21\nryan 30", "name alice ryan\nage 21 30"},
            {"a b c\nd e f", "a d\nb e\nc f"},
            {"single_word", "single_word"},
            {"1 2 3 4 5", "1\n2\n3\n4\n5"},
            {"", ""}
        };

        String testFileName = "file.txt";

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            try {
                String content = testCases[i][0];
                String expected = testCases[i][1];
                
                createTestFile(testFileName, content);
                String result = transposeFile(testFileName);
                
                System.out.println("Input:\n" + (content.isEmpty() ? "<empty file>" : content));
                System.out.println("\nOutput:\n" + result);
                System.out.println("\nExpected:\n" + expected);
                System.out.println("Test Passed: " + expected.equals(result));
                
                Files.delete(Paths.get(testFileName));
            } catch (IOException e) {
                System.err.println("Test case " + (i + 1) + " failed with an exception: " + e.getMessage());
            }
            System.out.println("--------------------");
        }
        
        System.out.println("--- Test Case 6 (File not found) ---");
        String result = transposeFile("non_existent_file.txt");
        String expected = "Error: File not found.";
        System.out.println("Output:\n" + result);
        System.out.println("\nExpected:\n" + expected);
        System.out.println("Test Passed: " + expected.equals(result));
        System.out.println("--------------------");
    }
}