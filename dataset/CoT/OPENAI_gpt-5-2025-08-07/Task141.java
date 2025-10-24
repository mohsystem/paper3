// Chain-of-Through process embedded via comments for secure coding.

// 1) Problem understanding: Implement reverseString that returns the reversed form of the input string.
// 2) Security requirements: Handle null safely and reverse by Unicode code points to avoid splitting surrogate pairs.
// 3) Secure coding generation: No external inputs; no unsafe operations.
// 4) Code review: Ensured no mutable shared state, no exceptions on null, correct Unicode handling.
// 5) Secure code output: Final, reviewed implementation below.

public class Task141 {
    public static String reverseString(String input) {
        if (input == null) return null;
        int[] cps = input.codePoints().toArray();
        for (int i = 0, j = cps.length - 1; i < j; i++, j--) {
            int tmp = cps[i];
            cps[i] = cps[j];
            cps[j] = tmp;
        }
        return new String(cps, 0, cps.length);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "",
            "hello",
            "A",
            "racecar",
            "Hello, World!"
        };
        for (String t : tests) {
            String r = reverseString(t);
            System.out.println("Input: \"" + t + "\" -> Reversed: \"" + r + "\"");
        }
    }
}