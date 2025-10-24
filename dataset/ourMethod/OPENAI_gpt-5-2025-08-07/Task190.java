import java.util.ArrayList;
import java.util.List;

public class Task190 {
    public static String transpose(String content) {
        if (content == null) throw new IllegalArgumentException("Input cannot be null");
        String trimmedAll = content.replace("\r", "").trim();
        if (trimmedAll.isEmpty()) return "";

        String[] rawLines = content.split("\n", -1);
        List<String> lines = new ArrayList<>();
        for (String ln : rawLines) {
            if (ln.endsWith("\r")) ln = ln.substring(0, ln.length() - 1);
            lines.add(ln);
        }

        int firstNonEmpty = -1, lastNonEmpty = -1;
        for (int i = 0; i < lines.size(); i++) {
            if (!lines.get(i).isEmpty()) {
                firstNonEmpty = i;
                break;
            }
        }
        for (int i = lines.size() - 1; i >= 0; i--) {
            if (!lines.get(i).isEmpty()) {
                lastNonEmpty = i;
                break;
            }
        }
        if (firstNonEmpty == -1) return "";
        for (int i = firstNonEmpty; i <= lastNonEmpty; i++) {
            if (lines.get(i).isEmpty()) {
                throw new IllegalArgumentException("Empty line detected within data block");
            }
        }

        List<String[]> rows = new ArrayList<>();
        int cols = -1;
        for (int i = firstNonEmpty; i <= lastNonEmpty; i++) {
            String line = lines.get(i);
            if (line.startsWith(" ") || line.endsWith(" ") || line.contains("  ")) {
                throw new IllegalArgumentException("Invalid spacing: leading/trailing or multiple consecutive spaces");
            }
            String[] tokens = line.isEmpty() ? new String[0] : line.split(" ", -1);
            if (cols == -1) cols = tokens.length;
            else if (tokens.length != cols) {
                throw new IllegalArgumentException("Inconsistent column counts");
            }
            rows.add(tokens);
        }

        if (rows.isEmpty()) return "";
        if (cols == 0) return "";

        StringBuilder sb = new StringBuilder();
        for (int c = 0; c < cols; c++) {
            for (int r = 0; r < rows.size(); r++) {
                sb.append(rows.get(r)[c]);
                if (r < rows.size() - 1) sb.append(' ');
            }
            if (c < cols - 1) sb.append('\n');
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "name age\nalice 21\nryan 30",
            "a b c",
            "a\nb\nc",
            "a b\nc d\n",
            "a b\nc"
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("Test " + (i + 1) + ":");
            try {
                String out = transpose(tests[i]);
                System.out.println(out);
            } catch (IllegalArgumentException ex) {
                System.out.println("ERROR: " + ex.getMessage());
            }
            System.out.println("----");
        }
    }
}