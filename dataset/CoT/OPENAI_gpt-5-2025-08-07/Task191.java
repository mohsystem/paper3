import java.util.ArrayDeque;
import java.util.Deque;

public class Task191 {
    public static int longestValidParentheses(String s) {
        if (s == null || s.isEmpty()) return 0;
        Deque<Integer> stack = new ArrayDeque<>();
        stack.push(-1);
        int maxLen = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '(') {
                stack.push(i);
            } else if (c == ')') {
                if (!stack.isEmpty()) stack.pop();
                if (stack.isEmpty()) {
                    stack.push(i);
                } else {
                    int len = i - stack.peek();
                    if (len > maxLen) maxLen = len;
                }
            }
        }
        return maxLen;
    }

    public static void main(String[] args) {
        String[] tests = {
            "(()",
            ")()())",
            "",
            "()(())",
            "((((("
        };
        for (String t : tests) {
            System.out.println(longestValidParentheses(t));
        }
    }
}