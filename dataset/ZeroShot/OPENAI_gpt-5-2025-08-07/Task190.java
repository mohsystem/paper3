import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task190 {
    public static List<String> transposeText(List<String> lines) {
        if (lines == null) throw new IllegalArgumentException("Input lines cannot be null");
        int n = lines.size();
        if (n == 0) return new ArrayList<>();

        List<List<String>> matrix = new ArrayList<>(n);
        int cols = -1;
        for (int i = 0; i < n; i++) {
            String line = lines.get(i);
            if (line == null) throw new IllegalArgumentException("Line " + i + " is null");
            String[] tokens = line.trim().split("\\s+");
            if (i == 0) {
                cols = tokens.length;
                if (cols == 0) throw new IllegalArgumentException("First line has no tokens");
            } else {
                if (tokens.length != cols) {
                    throw new IllegalArgumentException("Row " + i + " has inconsistent number of columns");
                }
            }
            matrix.add(Arrays.asList(tokens));
        }

        List<String> result = new ArrayList<>(cols);
        for (int c = 0; c < cols; c++) {
            StringBuilder sb = new StringBuilder();
            for (int r = 0; r < n; r++) {
                if (r > 0) sb.append(' ');
                sb.append(matrix.get(r).get(c));
            }
            result.add(sb.toString());
        }
        return result;
    }

    private static void runTest(int idx, List<String> input) {
        List<String> out = transposeText(input);
        System.out.println("Test " + idx + " output:");
        for (String s : out) System.out.println(s);
        System.out.println("---");
    }

    public static void main(String[] args) {
        // Test 1: Sample
        runTest(1, Arrays.asList(
                "name age",
                "alice 21",
                "ryan 30"
        ));

        // Test 2: Single row
        runTest(2, Arrays.asList(
                "a b c"
        ));

        // Test 3: Single column
        runTest(3, Arrays.asList(
                "a",
                "b",
                "c"
        ));

        // Test 4: Multiple spaces and trims
        runTest(4, Arrays.asList(
                "x   y",
                " 1    2 "
        ));

        // Test 5: 2x2
        runTest(5, Arrays.asList(
                "p q",
                "r s"
        ));
    }
}