import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class Task25 {

    /**
     * Prepends line numbers to a list of strings.
     * The numbering starts at 1. The format is n: string.
     *
     * @param lines The list of strings to be numbered. Can be null.
     * @return A new list of strings with each line prepended by its number,
     *         or an empty list if the input is null or empty.
     */
    public static List<String> number(List<String> lines) {
        if (lines == null || lines.isEmpty()) {
            return Collections.emptyList();
        }

        List<String> numberedLines = new ArrayList<>();
        int lineNumber = 1;
        for (String line : lines) {
            numberedLines.add(lineNumber + ": " + line);
            lineNumber++;
        }
        return numberedLines;
    }

    public static void main(String[] args) {
        // Test Case 1: Empty list
        List<String> test1 = new ArrayList<>();
        System.out.println("Test Case 1 Input: " + test1);
        System.out.println("Output: " + number(test1));
        System.out.println();

        // Test Case 2: Simple list
        List<String> test2 = Arrays.asList("a", "b", "c");
        System.out.println("Test Case 2 Input: " + test2);
        System.out.println("Output: " + number(test2));
        System.out.println();

        // Test Case 3: List with an empty string
        List<String> test3 = Arrays.asList("", "hello", "");
        System.out.println("Test Case 3 Input: " + test3);
        System.out.println("Output: " + number(test3));
        System.out.println();

        // Test Case 4: List with longer strings
        List<String> test4 = Arrays.asList("First line", "Second line", "Third line");
        System.out.println("Test Case 4 Input: " + test4);
        System.out.println("Output: " + number(test4));
        System.out.println();
        
        // Test Case 5: Null input
        List<String> test5 = null;
        System.out.println("Test Case 5 Input: " + test5);
        System.out.println("Output: " + number(test5));
        System.out.println();
    }
}