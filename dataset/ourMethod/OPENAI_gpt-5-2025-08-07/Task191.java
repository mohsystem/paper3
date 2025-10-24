import java.util.ArrayDeque;
import java.util.Deque;

public class Task191 {
    public static int longestValidParentheses(String s) {
        if (s == null) {
            return -1;
        }
        int n = s.length();
        if (n < 0 || n > 30000) {
            return -1;
        }
        for (int i = 0; i < n; i++) {
            char c = s.charAt(i);
            if (c != '(' && c != ')') {
                return -1;
            }
        }
        Deque<Integer> stack = new ArrayDeque<>();
        stack.push(-1);
        int maxLen = 0;
        for (int i = 0; i < n; i++) {
            char c = s.charAt(i);
            if (c == '(') {
                stack.push(i);
            } else {
                if (!stack.isEmpty()) {
                    stack.pop();
                }
                if (stack.isEmpty()) {
                    stack.push(i);
                } else {
                    int len = i - stack.peek();
                    if (len > maxLen) {
                        maxLen = len;
                    }
                }
            }
        }
        return maxLen;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "(()",
            ")()())",
            "",
            "()(())",
            "abc"
        };
        for (String t : tests) {
            int res = longestValidParentheses(t);
            System.out.println(res);
        }
    }
}