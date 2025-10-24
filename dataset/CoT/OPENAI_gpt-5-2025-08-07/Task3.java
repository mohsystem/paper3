// Chain-of-Through Step 1: Problem understanding - detect if a string is a pangram (contains every letter a-z at least once), case-insensitive, ignoring non-letters.
// Chain-of-Through Step 2: Security requirements - avoid unsafe operations, handle null input safely, limit processing to ASCII a-z.
// Chain-of-Through Step 3: Secure coding generation - careful character handling and bounds checks.
// Chain-of-Through Step 4: Code review - ensure no vulnerabilities like out-of-bounds or locale issues.
// Chain-of-Through Step 5: Secure code output - finalized, safe implementation.

public class Task3 {
    public static boolean isPangram(String s) {
        if (s == null) return false;
        boolean[] seen = new boolean[26];
        int count = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = Character.toLowerCase(s.charAt(i));
            if (c >= 'a' && c <= 'z') {
                int idx = c - 'a';
                if (!seen[idx]) {
                    seen[idx] = true;
                    count++;
                    if (count == 26) return true;
                }
            }
        }
        return count == 26;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "The quick brown fox jumps over the lazy dog",
            "Hello, world!",
            "",
            "Sphinx of black quartz, judge my vow 123!!!",
            "abcdefghijklmno pqrstuvwxy"
        };
        for (String t : tests) {
            boolean res = isPangram(t);
            System.out.println(res ? "True" : "False");
        }
    }
}