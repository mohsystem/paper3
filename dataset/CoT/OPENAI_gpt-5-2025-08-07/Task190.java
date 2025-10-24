import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task190 {

    // Transpose function: accepts list of lines, returns transposed content as a single string
    public static String transpose(List<String> lines) {
        if (lines == null) throw new IllegalArgumentException("Input lines cannot be null");
        if (lines.isEmpty()) return "";

        // Tokenize input lines using one or more spaces as delimiter
        List<String[]> matrix = new ArrayList<>(lines.size());
        int cols = -1;
        for (String line : lines) {
            if (line == null) throw new IllegalArgumentException("A line is null");
            String trimmed = line.trim();
            if (trimmed.isEmpty()) throw new IllegalArgumentException("Empty line not allowed");
            String[] tokens = trimmed.split(" +");
            if (cols == -1) cols = tokens.length;
            else if (cols != tokens.length) throw new IllegalArgumentException("Inconsistent column count");
            matrix.add(tokens);
        }

        int rows = matrix.size();
        StringBuilder out = new StringBuilder();
        for (int c = 0; c < cols; c++) {
            for (int r = 0; r < rows; r++) {
                out.append(matrix.get(r)[c]);
                if (r < rows - 1) out.append(' ');
            }
            if (c < cols - 1) out.append('\n');
        }
        return out.toString();
    }

    public static void main(String[] args) {
        // 5 test cases

        // 1) Example from prompt
        List<String> tc1 = Arrays.asList(
                "name age",
                "alice 21",
                "ryan 30"
        );
        System.out.println("TC1:\n" + transpose(tc1) + "\n");

        // 2) 1x1
        List<String> tc2 = Arrays.asList("a");
        System.out.println("TC2:\n" + transpose(tc2) + "\n");

        // 3) 1x3
        List<String> tc3 = Arrays.asList("x y z");
        System.out.println("TC3:\n" + transpose(tc3) + "\n");

        // 4) 3x1
        List<String> tc4 = Arrays.asList("x", "y", "z");
        System.out.println("TC4:\n" + transpose(tc4) + "\n");

        // 5) 3x3
        List<String> tc5 = Arrays.asList(
                "a b c",
                "d e f",
                "g h i"
        );
        System.out.println("TC5:\n" + transpose(tc5) + "\n");
    }
}