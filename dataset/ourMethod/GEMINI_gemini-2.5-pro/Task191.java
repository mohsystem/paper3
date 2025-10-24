import java.util.Stack;

public class Task191 {

    /**
     * Given a string containing just the characters '(' and ')', returns the length
     * of the longest valid (well-formed) parentheses substring.
     *
     * @param s The input string consisting of '(' and ')'.
     * @return The length of the longest valid parentheses substring.
     */
    public int longestValidParentheses(String s) {
        if (s == null || s.length() < 2) {
            return 0;
        }

        int maxLen = 0;
        Stack<Integer> stack = new Stack<>();
        // Push -1 as a sentinel value. It acts as a boundary for the first valid substring.
        stack.push(-1); 

        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) == '(') {
                stack.push(i);
            } else { // s.charAt(i) == ')'
                stack.pop();
                if (stack.isEmpty()) {
                    // Current ')' is unmatched. Push its index as the new boundary.
                    stack.push(i);
                } else {
                    // A valid pair is found. The length is the current index minus the
                    // index of the character just before the start of this valid substring.
                    maxLen = Math.max(maxLen, i - stack.peek());
                }
            }
        }
        return maxLen;
    }

    public static void main(String[] args) {
        Task191 solution = new Task191();

        // Test Case 1
        String s1 = "(()";
        System.out.println("Input: \"" + s1 + "\", Output: " + solution.longestValidParentheses(s1));

        // Test Case 2
        String s2 = ")()())";
        System.out.println("Input: \"" + s2 + "\", Output: " + solution.longestValidParentheses(s2));

        // Test Case 3
        String s3 = "";
        System.out.println("Input: \"" + s3 + "\", Output: " + solution.longestValidParentheses(s3));
        
        // Test Case 4
        String s4 = "()(()";
        System.out.println("Input: \"" + s4 + "\", Output: " + solution.longestValidParentheses(s4));
        
        // Test Case 5
        String s5 = "()(())";
        System.out.println("Input: \"" + s5 + "\", Output: " + solution.longestValidParentheses(s5));
    }
}