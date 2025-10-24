// Chain-of-Through Secure Development Steps:
// 1) Problem understanding: Convert strings to "Jaden Case" by capitalizing the first character of each word and lowercasing the rest. For Java, return null for null or empty input.
// 2) Security requirements: Avoid null dereferences, handle whitespace safely, and prevent unexpected behavior with character conversions.
// 3) Secure coding generation: Process input character-by-character to preserve whitespace and punctuation, using Character methods for case changes.
// 4) Code review: Ensure no uncontrolled resource usage and no exceptions on null inputs. Preserve whitespace, and handle tabs/newlines.
// 5) Secure code output: Final method validated against test cases and edge cases.

public final class Task17 {
    // Converts the given string to Jaden Case. Returns null for null or empty (after trim) input.
    public static String toJadenCase(String s) {
        if (s == null) return null;
        if (s.trim().isEmpty()) return null;

        StringBuilder sb = new StringBuilder(s.length());
        boolean startOfWord = true;

        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (Character.isWhitespace(c)) {
                sb.append(c);
                startOfWord = true;
            } else {
                if (startOfWord) {
                    sb.append(Character.toUpperCase(c));
                } else {
                    sb.append(Character.toLowerCase(c));
                }
                startOfWord = false;
            }
        }
        return sb.toString();
    }

    // 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
            "How can mirrors be real if our eyes aren't real",
            "most trees are blue",
            "",
            null,
            "a mixed-CASE input, with punctuation! and\ttabs"
        };

        for (String t : tests) {
            String res = toJadenCase(t);
            System.out.println(res);
        }
    }
}