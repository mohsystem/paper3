import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task119 {

    /**
     * Parses a string in CSV format into a list of lists of strings.
     * This is a simple parser and does not handle quoted fields.
     *
     * @param csvData The string containing CSV data.
     * @return A List of Lists, where each inner list represents a row.
     */
    public static List<List<String>> parseCSV(String csvData) {
        List<List<String>> records = new ArrayList<>();
        if (csvData == null || csvData.isEmpty()) {
            return records;
        }

        // Use \\R to split by any Unicode newline sequence
        String[] rows = csvData.split("\\R");
        for (String row : rows) {
            // Using -1 limit to include trailing empty strings
            String[] fields = row.split(",", -1);
            records.add(new ArrayList<>(Arrays.asList(fields)));
        }
        return records;
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Standard CSV
            "a,b,c\n1,2,3\nx,y,z",
            // Test Case 2: Empty fields and trailing commas
            "a,,c\n1,2,\n,y,z",
            // Test Case 3: Single row
            "one,two,three,four",
            // Test Case 4: Single column
            "one\ntwo\nthree",
            // Test Case 5: Empty input
            ""
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input:\n\"" + testCases[i] + "\"");
            List<List<String>> parsedData = parseCSV(testCases[i]);
            System.out.println("Output:");
            if (parsedData.isEmpty()) {
                System.out.println("[]");
            } else {
                for (List<String> row : parsedData) {
                    System.out.println(row);
                }
            }
            System.out.println();
        }
    }
}