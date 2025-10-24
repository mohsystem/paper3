// Task168 - Java implementation
// Chain-of-Through process in code generation:
// 1) Problem understanding: Shift each lowercase letter by corresponding array offset with wrap-around.
// 2) Security requirements: Validate indices, avoid out-of-bounds, handle nulls safely.
// 3) Secure coding generation: Use immutable String handling via StringBuilder, bounds checks.
// 4) Code review: Ensure modulo wrap-around for negatives, ignore non-lowercase safely.
// 5) Secure code output: Finalized robust solution with simple test cases.

import java.util.Arrays;

public class Task168 {
    public static String tweakLetters(String s, int[] arr) {
        if (s == null) return "";
        if (arr == null) arr = new int[0];
        StringBuilder sb = new StringBuilder(s.length());
        int n = s.length();
        int m = arr.length;
        for (int i = 0; i < n; i++) {
            char ch = s.charAt(i);
            if (ch >= 'a' && ch <= 'z' && i < m) {
                int base = ch - 'a';
                int shift = arr[i] % 26;
                if (shift < 0) shift += 26;
                int ni = (base + shift) % 26;
                sb.append((char) ('a' + ni));
            } else if (ch >= 'a' && ch <= 'z') {
                // No corresponding shift, keep as is
                sb.append(ch);
            } else {
                // Non-lowercase: leave unchanged per note
                sb.append(ch);
            }
        }
        return sb.toString();
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        System.out.println(tweakLetters("apple", new int[]{0, 1, -1, 0, -1})); // aqold
        System.out.println(tweakLetters("many", new int[]{0, 0, 0, -1})); // manx
        System.out.println(tweakLetters("rhino", new int[]{1, 1, 1, 1, 1})); // sijop
        System.out.println(tweakLetters("zzz", new int[]{1, 1, 1})); // aaa
        System.out.println(tweakLetters("abc", new int[]{-1, -1, -1})); // zab
    }
}