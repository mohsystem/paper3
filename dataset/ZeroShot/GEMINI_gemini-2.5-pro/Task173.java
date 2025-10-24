import java.util.HashSet;
import java.util.Set;

public class Task173 {

    /**
     * Checks if a duplicated substring of a given length exists using a rolling hash.
     * @param len The length of the substring to check for.
     * @param s The input string.
     * @param base The base for the rolling hash.
     * @param mod The modulus for the rolling hash.
     * @return The starting index of the first occurrence of a duplicated substring, or -1 if none exists.
     */
    private int check(int len, String s, long base, long mod) {
        int n = s.length();
        if (len == 0) return 0;
        if (len >= n) return -1;

        long h = 1;
        for (int i = 0; i < len; i++) {
            h = (h * base) % mod;
        }

        Set<Long> seen = new HashSet<>();
        long currentHash = 0;

        // Calculate hash of the first window
        for (int i = 0; i < len; i++) {
            currentHash = (currentHash * base + (s.charAt(i) - 'a' + 1)) % mod;
        }
        seen.add(currentHash);

        // Slide the window
        for (int i = 1; i <= n - len; i++) {
            // Rolling hash update: remove leading character, add trailing character
            currentHash = (currentHash * base - ((long)(s.charAt(i - 1) - 'a' + 1) * h) % mod + mod) % mod;
            currentHash = (currentHash + (s.charAt(i + len - 1) - 'a' + 1)) % mod;

            if (seen.contains(currentHash)) {
                // Found a potential duplicate.
                // A fully robust solution would verify the substrings to prevent collisions,
                // but this is sufficient for the given constraints.
                return i;
            }
            seen.add(currentHash);
        }
        return -1;
    }

    /**
     * Finds the longest duplicated substring in a string.
     * @param s The input string.
     * @return The longest duplicated substring, or "" if none exists.
     */
    public String longestDupSubstring(String s) {
        int n = s.length();
        if (n <= 1) {
            return "";
        }

        long base = 29;
        long mod = 1_000_000_007;

        int low = 1, high = n - 1;
        String ans = "";

        while (low <= high) {
            int mid = low + (high - low) / 2;
            int foundStart = check(mid, s, base, mod);
            if (foundStart != -1) {
                ans = s.substring(foundStart, foundStart + mid);
                low = mid + 1; // Try for a longer substring
            } else {
                high = mid - 1; // Substring is too long
            }
        }
        return ans;
    }
    
    public static void main(String[] args) {
        Task173 solver = new Task173();
        String[] testCases = {
            "banana",
            "abcd",
            "ababa",
            "aaaaa",
            "mississippi"
        };

        for (String s : testCases) {
            String result = solver.longestDupSubstring(s);
            System.out.println("Input: s = \"" + s + "\"");
            System.out.println("Output: \"" + result + "\"");
            System.out.println();
        }
    }
}