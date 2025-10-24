import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task190 {

    /**
     * Transposes the content of a text file.
     *
     * @param filePath The path to the input text file.
     */
    public static void transposeFile(String filePath) {
        try {
            List<String> lines = Files.readAllLines(Paths.get(filePath));
            if (lines.isEmpty()) {
                return;
            }

            List<List<String>> matrix = new ArrayList<>();
            for (String line : lines) {
                // Ignore empty lines that might exist in the file
                if (line.trim().isEmpty()) {
                    continue;
                }
                String[] words = line.trim().split("\\s+");
                matrix.add(Arrays.asList(words));
            }

            if (matrix.isEmpty() || matrix.get(0).isEmpty()) {
                return;
            }

            int numRows = matrix.size();
            int numCols = matrix.get(0).size();

            for (int j = 0; j < numCols; j++) {
                StringBuilder transposedRow = new StringBuilder();
                for (int i = 0; i < numRows; i++) {
                    transposedRow.append(matrix.get(i).get(j));
                    if (i < numRows - 1) {
                        transposedRow.append(" ");
                    }
                }
                System.out.println(transposedRow.toString());
            }

        } catch (IOException e) {
            System.err.println("Error processing file: " + e.getMessage());
        }
    }

    /**
     * Helper method to create a test file with given content.
     */
    private static void createTestFile(String fileName, String content) throws IOException {
        Files.write(Paths.get(fileName), content.getBytes());
    }

    public static void main(String[] args) {
        String[] testContents = {
            // Test Case 1: Example from prompt
            "name age\nalice 21\nryan 30",
            // Test Case 2: Single row
            "a b c d",
            // Test Case 3: Single column
            "a\nb\nc",
            // Test Case 4: 4x4 matrix
            "1 2 3 4\n5 6 7 8\n9 10 11 12\n13 14 15 16",
            // Test Case 5: 3x3 with strings
            "first middle last\njohn f kennedy\nmartin luther king"
        };

        String fileName = "file.txt";
        try {
            for (int i = 0; i < testContents.length; i++) {
                System.out.println("--- Test Case " + (i + 1) + " ---");
                System.out.println("Input:");
                System.out.println(testContents[i]);
                System.out.println("\nOutput:");
                createTestFile(fileName, testContents[i]);
                transposeFile(fileName);
                System.out.println();
            }
        } catch (IOException e) {
            System.err.println("Error during testing: " + e.getMessage());
        } finally {
            // Clean up the created file
            try {
                Files.deleteIfExists(Paths.get(fileName));
            } catch (IOException e) {
                // ignore
            }
        }
    }
}