import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task119 {

    public static List<List<String>> parseCSV(String input, char delimiter) {
        if (input == null) throw new IllegalArgumentException("input must not be null");
        List<List<String>> rows = new ArrayList<>();
        List<String> row = new ArrayList<>();
        StringBuilder field = new StringBuilder();
        boolean inQuotes = false;
        final int n = input.length();

        for (int i = 0; i < n; i++) {
            char c = input.charAt(i);
            if (inQuotes) {
                if (c == '"') {
                    if (i + 1 < n && input.charAt(i + 1) == '"') {
                        field.append('"');
                        i++;
                    } else {
                        inQuotes = false;
                    }
                } else {
                    field.append(c);
                }
            } else {
                if (c == '"') {
                    inQuotes = true;
                } else if (c == delimiter) {
                    row.add(field.toString());
                    field.setLength(0);
                } else if (c == '\n' || c == '\r') {
                    // Handle CRLF as single line break
                    if (c == '\r' && i + 1 < n && input.charAt(i + 1) == '\n') {
                        i++;
                    }
                    row.add(field.toString());
                    field.setLength(0);
                    rows.add(row);
                    row = new ArrayList<>();
                } else {
                    field.append(c);
                }
            }
        }
        // finalize last field/row
        row.add(field.toString());
        if (!(row.size() == 1 && row.get(0).isEmpty() && rows.isEmpty())) {
            rows.add(row);
        }
        return rows;
    }

    public static String toCSV(List<List<String>> rows, char delimiter) {
        if (rows == null) throw new IllegalArgumentException("rows must not be null");
        StringBuilder out = new StringBuilder();
        for (int r = 0; r < rows.size(); r++) {
            List<String> row = rows.get(r);
            for (int c = 0; c < row.size(); c++) {
                String field = row.get(c);
                if (field == null) field = "";
                boolean mustQuote = field.indexOf(delimiter) >= 0 || field.indexOf('"') >= 0 || field.indexOf('\n') >= 0 || field.indexOf('\r') >= 0;
                if (mustQuote) {
                    out.append('"');
                    for (int i = 0; i < field.length(); i++) {
                        char ch = field.charAt(i);
                        if (ch == '"') out.append("\"\"");
                        else out.append(ch);
                    }
                    out.append('"');
                } else {
                    out.append(field);
                }
                if (c + 1 < row.size()) out.append(delimiter);
            }
            if (r + 1 < rows.size()) out.append('\n');
        }
        return out.toString();
    }

    public static List<List<String>> selectColumns(List<List<String>> rows, int[] indices) {
        if (rows == null || indices == null) throw new IllegalArgumentException("rows/indices must not be null");
        // Validate indices: non-negative
        for (int idx : indices) {
            if (idx < 0) throw new IllegalArgumentException("column indices must be non-negative");
        }
        List<List<String>> out = new ArrayList<>(rows.size());
        for (List<String> row : rows) {
            List<String> newRow = new ArrayList<>(indices.length);
            for (int idx : indices) {
                String val = idx < row.size() ? row.get(idx) : "";
                newRow.add(val == null ? "" : val);
            }
            out.add(newRow);
        }
        return out;
    }

    public static double sumColumn(List<List<String>> rows, int colIndex) {
        if (rows == null) throw new IllegalArgumentException("rows must not be null");
        if (colIndex < 0) throw new IllegalArgumentException("colIndex must be non-negative");
        double sum = 0.0;
        for (List<String> row : rows) {
            if (colIndex < row.size()) {
                String s = row.get(colIndex);
                if (s != null) {
                    String t = s.trim();
                    if (!t.isEmpty()) {
                        try {
                            sum += Double.parseDouble(t);
                        } catch (NumberFormatException ignored) {
                        }
                    }
                }
            }
        }
        return sum;
    }

    private static void printRows(List<List<String>> rows) {
        System.out.println("Rows: " + rows.size());
        for (List<String> row : rows) {
            System.out.println(row);
        }
    }

    public static void main(String[] args) {
        // Test 1: Simple CSV
        String csv1 = "a,b\nc,d";
        List<List<String>> rows1 = parseCSV(csv1, ',');
        System.out.println("Test1 parsed:");
        printRows(rows1);
        System.out.println("Test1 serialized:");
        System.out.println(toCSV(rows1, ','));
        System.out.println();

        // Test 2: Quoted with comma
        String csv2 = "name,age\n\"Doe, John\",42\n\"Smith, Jane\",35";
        List<List<String>> rows2 = parseCSV(csv2, ',');
        System.out.println("Test2 parsed:");
        printRows(rows2);
        System.out.println("Sum age: " + sumColumn(rows2.subList(1, rows2.size()), 1)); // exclude header
        System.out.println();

        // Test 3: Newlines in quoted fields and escaped quotes
        String csv3 = "id,notes\n1,\"Line1\nLine2\"\n2,\"He said \"\"Hello\"\"\"";
        List<List<String>> rows3 = parseCSV(csv3, ',');
        System.out.println("Test3 parsed:");
        printRows(rows3);
        System.out.println("Test3 serialized:");
        System.out.println(toCSV(rows3, ','));
        System.out.println();

        // Test 4: Empty fields and trailing delimiters
        String csv4 = "a,,c\n,d,\n,,";
        List<List<String>> rows4 = parseCSV(csv4, ',');
        System.out.println("Test4 parsed:");
        printRows(rows4);
        List<List<String>> sel4 = selectColumns(rows4, new int[]{0, 2});
        System.out.println("Test4 select col 0 and 2:");
        printRows(sel4);
        System.out.println();

        // Test 5: Custom delimiter and CRLF
        String csv5 = "x;y\r\n1;2\r\n\"3;4\";5";
        List<List<String>> rows5 = parseCSV(csv5, ';');
        System.out.println("Test5 parsed:");
        printRows(rows5);
        System.out.println("Test5 serialized with ;:");
        System.out.println(toCSV(rows5, ';'));
    }
}