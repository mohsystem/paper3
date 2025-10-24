import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task190 {

    /**
     * Reads a space-delimited text file, transposes its content, and returns the result as a string.
     * Assumes all rows have the same number of columns.
     *
     * @param filename The path to the input file.
     * @return A string containing the transposed content, or null on error.
     */
    public static String transposeFileContent(String filename) {
        List<List<String>> grid = new ArrayList<>();
        try {
            // Secure: Using try-with-resources or explicit finally is not needed with readAllLines.
            // It handles file closing internally. Path traversal is not handled here, assumes a safe filename.
            List<String> lines = Files.readAllLines(Paths.get(filename));
            if (lines.isEmpty()) {
                return "";
            }

            for (String line : lines) {
                // Using a limit of -1 ensures that trailing empty strings are not discarded,
                // which is important for maintaining the grid structure if lines end with delimiters.
                String[] parts = line.split(" ", -1);
                grid.add(Arrays.asList(parts));
            }
        } catch (IOException e) {
            // Secure: Log errors instead of printing stack traces to production console.
            // Return null or throw a custom exception to signal failure.
            System.err.println("Error reading file: " + e.getMessage());
            return null;
        }

        if (grid.isEmpty() || grid.get(0).isEmpty()) {
            return "";
        }

        int numRows = grid.size();
        int numCols = grid.get(0).size();

        StringBuilder transposedContent = new StringBuilder();
        for (int j = 0; j < numCols; j++) {
            for (int i = 0; i < numRows; i++) {
                // Assumption from prompt: all rows have the same number of columns.
                // A robust solution would add checks for grid.get(i).size().
                transposedContent.append(grid.get(i).get(j));
                if (i < numRows - 1) {
                    transposedContent.append(" ");
                }
            }
            if (j < numCols - 1) {
                transposedContent.append("\n");
            }
        }
        return transposedContent.toString();
    }

    // Helper method to create a test file and run a test case
    private static void runTestCase(int testNum, String fileContent, String expectedOutput) {
        String filename = "file.txt";
        System.out.println("--- Test Case " + testNum + " ---");
        try {
            Files.write(Paths.get(filename), fileContent.getBytes());
            String result = transposeFileContent(filename);
            System.out.println("Input:\n" + (fileContent.isEmpty() ? "(empty)" : fileContent));
            System.out.println("\nOutput:\n" + (result == null || result.isEmpty() ? "(empty)" : result));
            System.out.println("\nExpected:\n" + (expectedOutput.isEmpty() ? "(empty)" : expectedOutput));
            System.out.println("\nResult matches expected: " + expectedOutput.equals(result));
        } catch (IOException e) {
            System.err.println("Test case failed with IO exception: " + e.getMessage());
        } finally {
            try {
                Files.deleteIfExists(Paths.get(filename));
            } catch (IOException e) {
                // Suppress cleanup errors
            }
        }
        System.out.println("---------------------\n");
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        runTestCase(1, "name age\nalice 21\nryan 30", "name alice ryan\nage 21 30");

        // Test Case 2: 1x4 Matrix (row vector)
        runTestCase(2, "a b c d", "a\nb\nc\nd");

        // Test Case 3: 3x1 Matrix (column vector)
        runTestCase(3, "x\ny\nz", "x y z");

        // Test Case 4: Square Matrix
        runTestCase(4, "1 2 3\n4 5 6\n7 8 9", "1 4 7\n2 5 8\n3 6 9");

        // Test Case 5: Empty File
        runTestCase(5, "", "");
    }
}