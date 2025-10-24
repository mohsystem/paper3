import java.util.ArrayList;
import java.util.List;

public class Task119 {
    public static List<List<String>> parseCSV(String input) {
        List<List<String>> rows = new ArrayList<>();
        if (input == null) return rows;
        StringBuilder field = new StringBuilder();
        List<String> curRow = new ArrayList<>();
        boolean inQuotes = false;
        boolean anyChar = false;
        boolean endedOnNewline = false;

        int i = 0;
        int n = input.length();
        while (i < n) {
            char c = input.charAt(i);
            anyChar = true;
            if (inQuotes) {
                if (c == '"') {
                    if (i + 1 < n && input.charAt(i + 1) == '"') {
                        field.append('"');
                        i += 2;
                    } else {
                        inQuotes = false;
                        i++;
                    }
                } else {
                    field.append(c);
                    i++;
                }
                endedOnNewline = false;
            } else {
                if (c == '"') {
                    inQuotes = true;
                    i++;
                    endedOnNewline = false;
                } else if (c == ',') {
                    curRow.add(field.toString());
                    field.setLength(0);
                    i++;
                    endedOnNewline = false;
                } else if (c == '\r' || c == '\n') {
                    curRow.add(field.toString());
                    field.setLength(0);
                    rows.add(curRow);
                    curRow = new ArrayList<>();
                    if (c == '\r' && i + 1 < n && input.charAt(i + 1) == '\n') i += 2;
                    else i++;
                    endedOnNewline = true;
                } else {
                    field.append(c);
                    i++;
                    endedOnNewline = false;
                }
            }
        }
        if (inQuotes || field.length() > 0 || !curRow.isEmpty() || (anyChar && !endedOnNewline)) {
            curRow.add(field.toString());
            rows.add(curRow);
        }
        return rows;
    }

    public static String toCSV(List<List<String>> rows) {
        if (rows == null) return "";
        StringBuilder out = new StringBuilder();
        for (int r = 0; r < rows.size(); r++) {
            List<String> row = rows.get(r);
            for (int c = 0; c < row.size(); c++) {
                String f = row.get(c);
                boolean needQuotes = false;
                for (int k = 0; k < f.length(); k++) {
                    char ch = f.charAt(k);
                    if (ch == '"' || ch == ',' || ch == '\n' || ch == '\r') {
                        needQuotes = true;
                        break;
                    }
                }
                if (needQuotes) {
                    out.append('"');
                    for (int k = 0; k < f.length(); k++) {
                        char ch = f.charAt(k);
                        if (ch == '"') out.append("\"\"");
                        else out.append(ch);
                    }
                    out.append('"');
                } else {
                    out.append(f);
                }
                if (c + 1 < row.size()) out.append(',');
            }
            if (r + 1 < rows.size()) out.append('\n');
        }
        return out.toString();
    }

    private static void printRows(String title, List<List<String>> rows) {
        System.out.println(title);
        for (int i = 0; i < rows.size(); i++) {
            System.out.print("Row " + i + " [");
            List<String> row = rows.get(i);
            for (int j = 0; j < row.size(); j++) {
                System.out.print((j == 0 ? "" : " | ") + row.get(j));
            }
            System.out.println("]");
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "a,b,c\n1,2,3",
            "name,quote\nJohn,\"Hello, world\"",
            "id,notes\n1,\"Line1\nLine2\"\n2,\"A\"",
            "text\n\"He said \"\"Hi\"\"\"",
            "a,b,c\r\n1,,3\r\n,2,\r\n"
        };

        for (int t = 0; t < tests.length; t++) {
            System.out.println("==== Test " + (t + 1) + " ====");
            String csv = tests[t];
            List<List<String>> parsed = parseCSV(csv);
            printRows("Parsed:", parsed);
            String serialized = toCSV(parsed);
            System.out.println("Serialized:");
            System.out.println(serialized);
            System.out.println();
        }
    }
}