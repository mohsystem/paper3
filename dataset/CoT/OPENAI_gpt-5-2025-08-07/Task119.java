// Step 1-5: Secure CSV parser with robust handling of quotes, commas, and newlines (RFC 4180-like)
import java.util.ArrayList;
import java.util.List;

public class Task119 {

    // Parse CSV content into list of records (each record is a list of fields)
    public static List<List<String>> parseCSV(String content) {
        return parseCSV(content, ',');
    }

    public static List<List<String>> parseCSV(String content, char delimiter) {
        List<List<String>> records = new ArrayList<>();
        if (content == null || content.isEmpty()) {
            return records;
        }

        List<String> row = new ArrayList<>();
        StringBuilder field = new StringBuilder();
        boolean inQuotes = false;

        final int n = content.length();
        for (int i = 0; i < n; i++) {
            char ch = content.charAt(i);
            if (inQuotes) {
                if (ch == '"') {
                    if (i + 1 < n && content.charAt(i + 1) == '"') {
                        field.append('"');
                        i++;
                    } else {
                        inQuotes = false;
                    }
                } else {
                    field.append(ch);
                }
            } else {
                if (ch == '"') {
                    inQuotes = true;
                } else if (ch == delimiter) {
                    row.add(field.toString());
                    field.setLength(0);
                } else if (ch == '\n' || ch == '\r') {
                    row.add(field.toString());
                    field.setLength(0);
                    records.add(row);
                    row = new ArrayList<>();
                    if (ch == '\r' && i + 1 < n && content.charAt(i + 1) == '\n') {
                        i++;
                    }
                } else {
                    field.append(ch);
                }
            }
        }

        // Finalize last field/row if applicable
        if (inQuotes || field.length() > 0 || !row.isEmpty()) {
            row.add(field.toString());
            records.add(row);
        }

        return records;
    }

    // Helper to print parsed CSV for demonstration
    private static void printParsed(String title, List<List<String>> recs) {
        System.out.println("=== " + title + " ===");
        for (int i = 0; i < recs.size(); i++) {
            List<String> row = recs.get(i);
            System.out.print("Row " + i + ": ");
            for (int j = 0; j < row.size(); j++) {
                String f = row.get(j);
                System.out.print("[" + f.replace("\n", "\\n").replace("\r", "\\r") + "]");
                if (j + 1 < row.size()) System.out.print(" | ");
            }
            System.out.println();
        }
        System.out.println();
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String t1 = "a,b,c\n1,2,3";
        String t2 = "Name,Note\n\"Smith, John\",\"He said \"\"Hello\"\".\"";
        String t3 = "A,B\n\"Line1\nLine2\",X";
        String t4 = "col1,col2,col3,col4\n, , ,\n,,";
        String t5 = "x,y\r\n1,2\r\n3,\"4\r\n5\",6";

        printParsed("Test 1", parseCSV(t1));
        printParsed("Test 2", parseCSV(t2));
        printParsed("Test 3", parseCSV(t3));
        printParsed("Test 4", parseCSV(t4));
        printParsed("Test 5", parseCSV(t5));
    }
}