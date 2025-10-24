// Chain-of-Through process:
// 1) Problem understanding: Determine if a string with only letters is an isogram (no repeating letters), case-insensitive; empty string is true.
// 2) Security requirements: Validate input, handle null safely, avoid locale pitfalls in case conversion, and ensure only letters are processed.
// 3) Secure coding generation: Implement with bounds-checked logic, use Locale.ROOT for case conversion, treat non-letters as invalid.
// 4) Code review: No external input, no dynamic memory, safe iteration over string, immediate return on invalid data or duplicates.
// 5) Secure code output: Final code reflects above safeguards.

public class Task4 {
    public static boolean isIsogram(String s) {
        if (s == null) {
            return true; // Treat null as isogram for safety; spec assumes only letters
        }
        String lower = s.toLowerCase(java.util.Locale.ROOT);
        boolean[] seen = new boolean[26];
        for (int i = 0; i < lower.length(); i++) {
            char c = lower.charAt(i);
            if (c >= 'a' && c <= 'z') {
                int idx = c - 'a';
                if (seen[idx]) {
                    return false;
                }
                seen[idx] = true;
            } else {
                // Non-letter violates the letters-only assumption
                return false;
            }
        }
        return true;
    }

    private static void runTest(String input) {
        boolean result = isIsogram(input);
        String display = (input == null) ? "null" : "\"" + input + "\"";
        System.out.println(display + " --> " + result);
    }

    public static void main(String[] args) {
        runTest("Dermatoglyphics"); // true
        runTest("aba");             // false
        runTest("moOse");           // false
        runTest("");                // true
        runTest("isogram");         // true
    }
}