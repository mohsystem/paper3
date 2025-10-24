import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task25 {

    /**
     * Takes a list of strings and returns each line prepended by the correct number.
     * The numbering starts at 1. The format is n: string.
     * @param lines The list of strings.
     * @return A new list of strings with line numbers.
     */
    public static List<String> number(List<String> lines) {
        if (lines == null) {
            return new ArrayList<>();
        }
        List<String> result = new ArrayList<>();
        for (int i = 0; i < lines.size(); i++) {
            result.add((i + 1) + ": " + lines.get(i));
        }
        return result;
    }

    public static void main(String[] args) {
        // Test Case 1: Basic case
        List<String> input1 = Arrays.asList("a", "b", "c");
        System.out.println("Test Case 1");
        System.out.println("Input: " + input1);
        System.out.println("Output: " + number(input1)); // Expected: [1: a, 2: b, 3: c]
        System.out.println();

        // Test Case 2: Empty list
        List<String> input2 = new ArrayList<>();
        System.out.println("Test Case 2");
        System.out.println("Input: " + input2);
        System.out.println("Output: " + number(input2)); // Expected: []
        System.out.println();
        
        // Test Case 3: List with empty strings
        List<String> input3 = Arrays.asList("", "x", "");
        System.out.println("Test Case 3");
        System.out.println("Input: " + input3);
        System.out.println("Output: " + number(input3)); // Expected: [1: , 2: x, 3: ]
        System.out.println();

        // Test Case 4: List with multi-word strings
        List<String> input4 = Arrays.asList("hello world", "goodbye world");
        System.out.println("Test Case 4");
        System.out.println("Input: " + input4);
        System.out.println("Output: " + number(input4)); // Expected: [1: hello world, 2: goodbye world]
        System.out.println();

        // Test Case 5: Single element list
        List<String> input5 = Arrays.asList("Just one line.");
        System.out.println("Test Case 5");
        System.out.println("Input: " + input5);
        System.out.println("Output: " + number(input5)); // Expected: [1: Just one line.]
        System.out.println();
    }
}