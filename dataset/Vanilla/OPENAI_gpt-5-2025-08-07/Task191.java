import java.util.*;

public class Task191 {
    public static int longestValidParentheses(String s) {
        Deque<Integer> st = new ArrayDeque<>();
        st.push(-1);
        int maxLen = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '(') {
                st.push(i);
            } else {
                st.pop();
                if (st.isEmpty()) {
                    st.push(i);
                } else {
                    maxLen = Math.max(maxLen, i - st.peek());
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
            "())((())"
        };
        for (String t : tests) {
            System.out.println("Input: \"" + t + "\" => Output: " + longestValidParentheses(t));
        }
    }
}