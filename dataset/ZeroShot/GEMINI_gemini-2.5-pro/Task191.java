import java.util.ArrayDeque;
import java.util.Deque;

public class Task191 {

    /**
     * Given a string containing just the characters '(' and ')', returns the length of the
     * longest valid (well-formed) parentheses substring.
     *
     * @param s The input string consisting of '(' and ')'.
     * @return The length of the longest valid parentheses substring.
     */
    public int longestValidParentheses(String s) {
        if (s == null || s.length() < 2) {
            return 0;
        }

        int maxLength = 0;
        // Use a stack to store indices of '(' characters.
        // ArrayDeque is generally preferred over Stack in modern Java.
        Deque<Integer> stack = new ArrayDeque<>();
        // Push -1 as a sentinel value to mark the base for a valid substring.
        stack.push(-1);

        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) == '(') {
                stack.push(i);
            } else { // s.charAt(i) == ')'
                stack.pop();
                if (stack.isEmpty()) {
                    // This ')' does not have a matching '('.
                    // Push its index to serve as the new base.
                    stack.push(i);
                } else {
                    // A valid pair is found.
                    // The length is the current index minus the index of the element
                    // at the top of the stack (which is the new base).
                    maxLength = Math.max(maxLength, i - stack.peek());
                }
            }
        }
        return maxLength;
    }

    public static void main(String[] args) {
        Task191 solution = new Task191();
        String[] testCases = {"(()", ")()())", "", "()(()", "((()))())"};
        int[] expectedResults = {2, 4, 0, 2, 8};

        for (int i = 0; i < testCases.length; i++) {
            int result = solution.longestValidParentheses(testCases[i]);
            System.out.println("Test Case " + (i + 1) + ": \"" + testCases[i] + "\"");
            System.out.println("Result: " + result);
            System.out.println("Expected: " + expectedResults[i]);
            System.out.println(result == expectedResults[i] ? "Passed" : "Failed");
            System.out.println();
        }
    }
}