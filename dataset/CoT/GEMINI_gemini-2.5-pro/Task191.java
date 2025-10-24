import java.util.Stack;

public class Task191 {

    /**
     * Given a string containing just the characters '(' and ')', returns the length of the longest 
     * valid (well-formed) parentheses substring.
     *
     * This method uses a stack to keep track of the indices of the parentheses.
     * The stack stores the indices of '(' characters. An initial -1 is pushed to the stack
     * to act as a base for calculating the length of a valid substring.
     *
     * When a '(' is encountered, its index is pushed onto the stack.
     * When a ')' is encountered, the top of the stack is popped.
     * - If the stack becomes empty after popping, it means the current ')' does not have a matching '('.
     *   So, the index of this ')' is pushed to serve as the new base for future valid substrings.
     * - If the stack is not empty after popping, it means a valid pair is formed. The length of the
     *   current valid substring is the difference between the current index and the index at the top of the stack.
     *   The maximum length is updated accordingly.
     *
     * @param s The input string containing only '(' and ')' characters.
     * @return The length of the longest valid parentheses substring.
     */
    public int longestValidParentheses(String s) {
        if (s == null || s.length() < 2) {
            return 0;
        }
        int maxLength = 0;
        Stack<Integer> stack = new Stack<>();
        stack.push(-1); // Base for the first valid substring

        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) == '(') {
                stack.push(i);
            } else { // s.charAt(i) == ')'
                stack.pop();
                if (stack.isEmpty()) {
                    // This ')' is unmatched, it becomes the new base
                    stack.push(i);
                } else {
                    // A match is found. Calculate the length from the new top of the stack.
                    maxLength = Math.max(maxLength, i - stack.peek());
                }
            }
        }
        return maxLength;
    }

    public static void main(String[] args) {
        Task191 solution = new Task191();
        
        // Test Case 1
        String s1 = "(()";
        System.out.println("Input: \"" + s1 + "\", Output: " + solution.longestValidParentheses(s1)); // Expected: 2

        // Test Case 2
        String s2 = ")()())";
        System.out.println("Input: \"" + s2 + "\", Output: " + solution.longestValidParentheses(s2)); // Expected: 4

        // Test Case 3
        String s3 = "";
        System.out.println("Input: \"" + s3 + "\", Output: " + solution.longestValidParentheses(s3)); // Expected: 0

        // Test Case 4
        String s4 = "()(()";
        System.out.println("Input: \"" + s4 + "\", Output: " + solution.longestValidParentheses(s4)); // Expected: 2

        // Test Case 5
        String s5 = "((()))";
        System.out.println("Input: \"" + s5 + "\", Output: " + solution.longestValidParentheses(s5)); // Expected: 6
    }
}