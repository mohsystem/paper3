import java.util.ArrayList;
import java.util.List;

public class Task119 {

    // Parses a CSV string into a list of rows, each row is a list of fields.
    public static List<List<String>> parseCSV(String csv) {
        List<List<String>> rows = new ArrayList<>();
        if (csv == null || csv.isEmpty()) {
            return rows;
        }

        List<String> currentRow = new ArrayList<>();
        StringBuilder field = new StringBuilder();
        boolean inQuotes = false;
        boolean fieldStart = true;

        final int n = csv.length();
        for (int i = 0; i < n; i++) {
            char ch = csv.charAt(i);
            if (inQuotes) {
                if (ch == '"') {
                    if (i + 1 < n && csv.charAt(i + 1) == '"') {
                        field.append('"');
                        i++;
                    } else {
                        inQuotes = false;
                    }
                } else {
                    field.append(ch);
                }
            } else {
                if (fieldStart && ch == '"') {
                    inQuotes = true;
                    fieldStart = false;
                } else if (ch == ',') {
                    currentRow.add(field.toString());
                    field.setLength(0);
                    fieldStart = true;
                } else if (ch == '\n') {
                    currentRow.add(field.toString());
                    field.setLength(0);
                    rows.add(currentRow);
                    currentRow = new ArrayList<>();
                    fieldStart = true;
                } else if (ch == '\r') {
                    // Handle CRLF or lone CR
                    if (i + 1 < n && csv.charAt(i + 1) == '\n') {
                        i++;
                    }
                    currentRow.add(field.toString());
                    field.setLength(0);
                    rows.add(currentRow);
                    currentRow = new ArrayList<>();
                    fieldStart = true;
                } else {
                    field.append(ch);
                    fieldStart = false;
                }
            }
        }
        // Finalize last field/row
        if (inQuotes) {
            // If unbalanced quotes, treat as end of field
        }
        // Add last field
        if (field.length() > 0 || !fieldStart || !currentRow.isEmpty() || (n > 0 && (csv.charAt(n - 1) == ','))) {
            currentRow.add(field.toString());
        }
        if (!currentRow.isEmpty()) {
            rows.add(currentRow);
        }
        return rows;
    }

    // Utility method to format parsed CSV for display
    public static String toDebugString(List<List<String>> data) {
        StringBuilder sb = new StringBuilder();
        sb.append("Rows=").append(data.size()).append('\n');
        for (int r = 0; r < data.size(); r++) {
            sb.append("Row ").append(r).append(": [");
            List<String> row = data.get(r);
            for (int c = 0; c < row.size(); c++) {
                String val = row.get(c);
                String safe = val.replace("\\", "\\\\").replace("\"", "\\\"");
                sb.append("\"").append(safe).append("\"");
                if (c + 1 < row.size()) sb.append(", ");
            }
            sb.append("]\n");
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "name,age,city\nAlice,30,Seattle",
            "a,\"b,b\",c",
            "row1col1,row1col2\r\nrow2col1,\"row2\ncol2\",row2col3",
            "\"He said \"\"Hello\"\"\",42",
            "1,2,3,\n, ,\"\""
        };
        for (int i = 0; i < tests.length; i++) {
            List<List<String>> parsed = parseCSV(tests[i]);
            System.out.println("Test " + (i + 1) + ":");
            System.out.print(toDebugString(parsed));
        }
    }
}