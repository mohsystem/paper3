import java.util.ArrayList;
import java.util.List;

public class Task190 {

    /**
     * Transposes the given content where rows are separated by newlines
     * and columns by spaces.
     *
     * @param content The string content to transpose.
     * @return The transposed string content.
     */
    public static String transposeContent(String content) {
        if (content == null || content.isEmpty()) {
            return "";
        }

        String[] lines = content.split("\n");
        if (lines.length == 0) {
            return "";
        }

        List<String[]> rows = new ArrayList<>();
        for (String line : lines) {
            // Skips any empty lines that might result from multiple newlines
            if (!line.trim().isEmpty()) {
                rows.add(line.split(" "));
            }
        }
        
        if (rows.isEmpty()) {
            return "";
        }

        int numRows = rows.size();
        // Assumes all rows have the same number of columns as the first one
        int numCols = rows.get(0).length;

        StringBuilder transposedContent = new StringBuilder();
        for (int j = 0; j < numCols; j++) {
            for (int i = 0; i < numRows; i++) {
                // This check adds robustness for potentially jagged inputs
                if (j < rows.get(i).length) {
                    transposedContent.append(rows.get(i)[j]);
                }
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

    public static void main(String[] args) {
        String[] testCases = {
            "name age\nalice 21\nryan 30",
            "a b c\nd e f\ng h i",
            "one two three four",
            "apple\nbanana\ncherry",
            ""
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input:");
            System.out.println(testCases[i].isEmpty() ? "<empty>" : testCases[i]);
            System.out.println("Output:");
            String result = transposeContent(testCases[i]);
            System.out.println(result.isEmpty() ? "<empty>" : result);
            System.out.println("--------------------");
        }
    }
}