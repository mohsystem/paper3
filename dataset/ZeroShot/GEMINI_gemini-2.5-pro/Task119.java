import java.util.ArrayList;
import java.util.List;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;

public class Task119 {

    /**
     * Securely parses a CSV string into a list of lists of strings.
     * This implementation handles quoted fields, escaped quotes, and processes the
     * input line by line to avoid loading a potentially large file into memory at once.
     * It gracefully handles malformed lines by parsing them as best as possible without crashing.
     *
     * @param csvContent The string content of the CSV file.
     * @return A List of Lists of Strings representing the CSV data.
     */
    public static List<List<String>> parseCSV(String csvContent) {
        List<List<String>> records = new ArrayList<>();
        if (csvContent == null || csvContent.trim().isEmpty()) {
            return records;
        }

        try (BufferedReader br = new BufferedReader(new StringReader(csvContent))) {
            String line;
            while ((line = br.readLine()) != null) {
                List<String> currentRow = new ArrayList<>();
                StringBuilder currentField = new StringBuilder();
                boolean inQuotes = false;
                
                for (int i = 0; i < line.length(); i++) {
                    char c = line.charAt(i);

                    if (inQuotes) {
                        if (c == '"') {
                            // Check for escaped quote ("")
                            if (i + 1 < line.length() && line.charAt(i + 1) == '"') {
                                currentField.append('"');
                                i++; // Skip next quote
                            } else {
                                inQuotes = false;
                            }
                        } else {
                            currentField.append(c);
                        }
                    } else {
                        if (c == '"') {
                            // Don't append the opening quote to the field
                            inQuotes = true;
                        } else if (c == ',') {
                            currentRow.add(currentField.toString());
                            currentField.setLength(0); // Reset for the next field
                        } else {
                            currentField.append(c);
                        }
                    }
                }
                currentRow.add(currentField.toString());
                records.add(currentRow);
            }
        } catch (IOException e) {
            // This is unlikely with StringReader, but good practice for file I/O
            System.err.println("Error reading CSV content: " + e.getMessage());
        }
        
        return records;
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Simple CSV
            "a,b,c\n1,2,3",
            
            // Test Case 2: Quoted fields with commas
            "a,\"b,c\",d\n1,\"hello, world\",3",
            
            // Test Case 3: Escaped quotes inside quoted fields
            "a,\"b\"\"c\",d\n1,\"it's \"\"ok\"\"\",3",
            
            // Test Case 4: Malformed/edge cases (inconsistent columns, empty lines)
            "a,b\n1,2,3\n\n,d",
            
            // Test Case 5: Complex case with mixed quoting and commas
            "Name,Address,Notes\n\"Doe, John\",\"120 Main St, \"\"Anytown\"\"\",\"He said, \"\"Hello!\"\"\""
        };
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input:\n" + testCases[i]);
            List<List<String>> result = parseCSV(testCases[i]);
            System.out.println("Output:");
            int rowNum = 0;
            for (List<String> row : result) {
                System.out.print("Row " + (rowNum++) + ": [");
                for (int j = 0; j < row.size(); j++) {
                    System.out.print("\"" + row.get(j) + "\"");
                    if (j < row.size() - 1) {
                        System.out.print(", ");
                    }
                }
                System.out.println("]");
            }
            System.out.println();
        }
    }
}