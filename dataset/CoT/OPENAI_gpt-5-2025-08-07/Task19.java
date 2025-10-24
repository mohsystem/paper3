// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement spinWords to reverse words of length >= 5 while preserving spaces.
// 2) Security requirements: Avoid unsafe operations, null checks, and handle inputs without modifying them unexpectedly.
// 3) Secure coding generation: Use bounds-checked loops and immutable input handling.
// 4) Code review: Ensure no buffer overflows or misuse of APIs.
// 5) Secure code output: Final code adheres to the above and includes tests.

public class Task19 {
    public static String spinWords(String s) {
        if (s == null) return null;
        StringBuilder out = new StringBuilder(s.length());
        int n = s.length();
        int i = 0;
        while (i < n) {
            if (s.charAt(i) == ' ') {
                out.append(' ');
                i++;
                continue;
            }
            int start = i;
            while (i < n && s.charAt(i) != ' ') i++;
            int len = i - start;
            if (len >= 5) {
                for (int j = i - 1; j >= start; j--) {
                    out.append(s.charAt(j));
                }
            } else {
                out.append(s, start, i);
            }
        }
        return out.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hey fellow warriors",
            "This is a test",
            "This is another test",
            "Welcome",
            "abcd efghi jklmn op qrstu vwxyz"
        };
        for (String t : tests) {
            System.out.println(spinWords(t));
        }
    }
}