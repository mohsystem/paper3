import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Collections;

public class Task25 {

    /**
     * Prepends line numbers to a list of strings.
     *
     * @param lines The list of strings to number.
     * @return A new list of strings with each line prepended by "n: ".
     */
    public static List<String> number(List<String> lines) {
        if (lines == null) {
            return Collections.emptyList();
        }

        List<String> numberedLines = new ArrayList<>();
        for (int i = 0; i < lines.size(); i++) {
            // The numbering starts at 1, so we use i + 1.
            numberedLines.add((i + 1) + ": " + lines.get(i));
        }
        return numberedLines;
    }

    public static void main(String[] args) {
        // Test Case 1: Empty list
        List<String> test1 = new ArrayList<>();
        System.out.println("Test Case 1 (Input: " + test1 + ")");
        System.out.println("Output: " + number(test1));
        System.out.println();

        // Test Case 2: Standard list
        List<String> test2 = Arrays.asList("a", "b", "c");
        System.out.println("Test Case 2 (Input: " + test2 + ")");
        System.out.println("Output: " + number(test2));
        System.out.println();

        // Test Case 3: List with empty strings
        List<String> test3 = Arrays.asList("", "", "d");
        System.out.println("Test Case 3 (Input: " + test3 + ")");
        System.out.println("Output: " + number(test3));
        System.out.println();

        // Test Case 4: List with two strings
        List<String> test4 = Arrays.asList("Hello", "World");
        System.out.println("Test Case 4 (Input: " + test4 + ")");
        System.out.println("Output: " + number(test4));
        System.out.println();

        // Test Case 5: List with one string
        List<String> test5 = Arrays.asList("single line");
        System.out.println("Test Case 5 (Input: " + test5 + ")");
        System.out.println("Output: " + number(test5));
        System.out.println();
    }
}