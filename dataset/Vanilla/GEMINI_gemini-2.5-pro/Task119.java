import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

public class Task119 {

    /**
     * Parses a CSV string into a list of lists of strings.
     * This implementation handles quoted fields containing commas and escaped double quotes.
     *
     * @param csvContent The string content of the CSV file.
     * @return A List of Lists of Strings representing the parsed CSV data.
     */
    public static List<List<String>> parseCsv(String csvContent) {
        List<List<String>> records = new ArrayList<>();
        if (csvContent == null || csvContent.isEmpty()) {
            return records;
        }

        String[] lines = csvContent.split("\n");
        for (String line : lines) {
            if (line.trim().isEmpty()) {
                continue;
            }
            List<String> currentRecord = new ArrayList<>();
            StringBuilder currentField = new StringBuilder();
            boolean inQuotes = false;
            
            for (int i = 0; i < line.length(); i++) {
                char c = line.charAt(i);

                if (inQuotes) {
                    if (c == '"') {
                        // Check for escaped quote ("")
                        if (i + 1 < line.length() && line.charAt(i + 1) == '"') {
                            currentField.append('"');
                            i++; // Skip the next quote
                        } else {
                            inQuotes = false;
                        }
                    } else {
                        currentField.append(c);
                    }
                } else {
                    if (c == '"') {
                        inQuotes = true;
                    } else if (c == ',') {
                        currentRecord.add(currentField.toString());
                        currentField.setLength(0); // Reset for the next field
                    } else {
                        currentField.append(c);
                    }
                }
            }
            currentRecord.add(currentField.toString());
            records.add(currentRecord);
        }
        return records;
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Simple CSV
            "a,b,c\n1,2,3",
            // Test Case 2: Quoted fields with commas
            "\"a,b\",c\n1,\"2,3\"",
            // Test Case 3: Empty fields and empty quoted fields
            "a,,c\n1,\"\",3",
            // Test Case 4: Escaped quotes within a quoted field
            "a,\"b\"\"c\",d\n\"e \"\"f\"\"\",g",
            // Test Case 5: Mixed and complex cases
            "Name,Age,\"Address, City\"\nJohn Doe,30,\"123 Main St, \"\"Anytown\"\"\"\n\"Jane, Smith\",25,\"456 Oak Ave, Somewhere\""
        };
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input:\n" + testCases[i]);
            List<List<String>> result = parseCsv(testCases[i]);
            System.out.println("Output:");
            for (List<String> row : result) {
                System.out.println(row);
            }
            System.out.println();
        }
    }
}