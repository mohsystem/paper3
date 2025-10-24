import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Arrays;

public class Task25 {

    /**
     * Prepends line numbers to a list of strings.
     * The numbering starts at 1. The format is n: string.
     *
     * @param lines The list of strings to be numbered. Can be null.
     * @return A new list of strings with line numbers, or an empty list if input is null or empty.
     */
    public static List<String> numberLines(List<String> lines) {
        if (lines == null || lines.isEmpty()) {
            return Collections.emptyList();
        }

        List<String> result = new ArrayList<>();
        int lineNumber = 1;
        for (String line : lines) {
            // Using StringBuilder is efficient for string concatenation in a loop.
            // It avoids creating multiple intermediate String objects.
            StringBuilder sb = new StringBuilder();
            sb.append(lineNumber);
            sb.append(": ");
            // Defensively handle potential nulls inside the list
            if (line != null) {
                sb.append(line);
            }
            result.add(sb.toString());
            lineNumber++;
        }
        return result;
    }

    public static void main(String[] args) {
        // Test Case 1: Empty list
        List<String> test1 = new ArrayList<>();
        System.out.println("Test Case 1: Input [] -> Output " + numberLines(test1));

        // Test Case 2: Basic list
        List<String> test2 = Arrays.asList("a", "b", "c");
        System.out.println("Test Case 2: Input [\"a\", \"b\", \"c\"] -> Output " + numberLines(test2));

        // Test Case 3: List with empty strings
        List<String> test3 = Arrays.asList("first", "", "third");
        System.out.println("Test Case 3: Input [\"first\", \"\", \"third\"] -> Output " + numberLines(test3));
        
        // Test Case 4: Single element list
        List<String> test4 = Arrays.asList("single line");
        System.out.println("Test Case 4: Input [\"single line\"] -> Output " + numberLines(test4));

        // Test Case 5: Null input list
        List<String> test5 = null;
        System.out.println("Test Case 5: Input null -> Output " + numberLines(test5));
    }
}