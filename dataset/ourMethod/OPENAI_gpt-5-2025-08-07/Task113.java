import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Task113 {
    private static final int MAX_PARTS = 1000;
    private static final int MAX_PART_LEN = 100_000;
    private static final int MAX_TOTAL_LEN = 1_000_000;

    public static String safeConcat(List<String> parts, int maxTotalLen, int maxPartLen) {
        if (parts == null) {
            throw new IllegalArgumentException("Input list must not be null.");
        }
        if (parts.size() > MAX_PARTS) {
            throw new IllegalArgumentException("Too many parts: " + parts.size());
        }
        long total = 0;
        for (String s : parts) {
            if (s == null) {
                throw new IllegalArgumentException("Null string element is not allowed.");
            }
            int len = s.length();
            if (len > maxPartLen) {
                throw new IllegalArgumentException("Part exceeds maximum allowed length.");
            }
            total += len;
            if (total > maxTotalLen) {
                throw new IllegalArgumentException("Total length exceeds maximum allowed length.");
            }
        }
        StringBuilder sb = new StringBuilder((int) total);
        for (String s : parts) {
            sb.append(s);
        }
        return sb.toString();
    }

    public static String safeConcat(List<String> parts) {
        return safeConcat(parts, MAX_TOTAL_LEN, MAX_PART_LEN);
    }

    private static void runTests() {
        // Test 1
        System.out.println(safeConcat(List.of("Hello", " ", "World")));

        // Test 2
        System.out.println(safeConcat(List.of("", "abc", "", "def")));

        // Test 3
        System.out.println(safeConcat(List.of("multi", "-", "part", "-", "concat")));

        // Test 4 (empty list)
        System.out.println(safeConcat(Collections.emptyList()));

        // Test 5 (Unicode)
        System.out.println(safeConcat(List.of("Safe ", "concat ", "テスト")));
    }

    private static void readFromUser() {
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            System.out.println("Enter number of strings to concatenate (0.." + MAX_PARTS + "):");
            String line = br.readLine();
            if (line == null) {
                return;
            }
            line = line.trim();
            if (line.isEmpty()) {
                System.err.println("ERROR: Empty input for count.");
                return;
            }
            int n;
            try {
                n = Integer.parseInt(line);
            } catch (NumberFormatException e) {
                System.err.println("ERROR: Invalid number format.");
                return;
            }
            if (n < 0 || n > MAX_PARTS) {
                System.err.println("ERROR: Count out of allowed range.");
                return;
            }
            List<String> parts = new ArrayList<>(n);
            long total = 0;
            for (int i = 0; i < n; i++) {
                String s = br.readLine();
                if (s == null) {
                    System.err.println("ERROR: Not enough lines provided.");
                    return;
                }
                if (s.length() > MAX_PART_LEN) {
                    System.err.println("ERROR: A part exceeded maximum length.");
                    return;
                }
                total += s.length();
                if (total > MAX_TOTAL_LEN) {
                    System.err.println("ERROR: Total length exceeded maximum allowed length.");
                    return;
                }
                parts.add(s);
            }
            String result = safeConcat(parts);
            System.out.println("Concatenated result:");
            System.out.println(result);
        } catch (IOException e) {
            System.err.println("ERROR: I/O error occurred.");
        } catch (IllegalArgumentException e) {
            System.err.println("ERROR: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        runTests();
        readFromUser();
    }
}