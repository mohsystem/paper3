import java.util.HashSet;
import java.util.Set;

public class Task173 {

    private String s;
    private long[] power;
    private final int base = 26;
    // A large prime modulus, 2^31 - 1
    private final long modulus = 2147483647;

    /**
     * Helper function to check if a duplicate substring of a given length exists.
     * Uses Rabin-Karp with rolling hash.
     * @param len The length of the substring to search for.
     * @return The starting index of the first found duplicate, or -1 if none exists.
     */
    private int search(int len) {
        int n = s.length();
        if (len == 0) return 0;
        if (len >= n) return -1;
        
        long currentHash = 0;
        // Calculate hash of the first substring of length `len`
        for (int i = 0; i < len; i++) {
            currentHash = (currentHash * base + (s.charAt(i) - 'a')) % modulus;
        }

        Set<Long> seen = new HashSet<>();
        seen.add(currentHash);

        long highPower = power[len - 1]; // base^(len-1) % modulus

        for (int i = 1; i <= n - len; i++) {
            // Rolling hash update: remove leading character, add trailing character
            currentHash = (currentHash - (long)(s.charAt(i - 1) - 'a') * highPower) % modulus;
            currentHash = (currentHash * base + (s.charAt(i + len - 1) - 'a')) % modulus;
            
            // Ensure hash is non-negative
            if (currentHash < 0) {
                currentHash += modulus;
            }

            if (seen.contains(currentHash)) {
                // A hash collision is possible but unlikely with a large modulus.
                // We assume no collision for simplicity and performance.
                return i;
            }
            seen.add(currentHash);
        }
        return -1;
    }

    /**
     * Finds the longest duplicated substring in a given string.
     * @param s The input string.
     * @return The longest duplicated substring, or "" if none exists.
     */
    public String longestDupSubstring(String s) {
        this.s = s;
        int n = s.length();
        if (n <= 1) return "";

        // Precompute powers of the base for rolling hash calculation
        power = new long[n];
        power[0] = 1;
        for (int i = 1; i < n; i++) {
            power[i] = (power[i - 1] * base) % modulus;
        }

        int low = 1, high = n - 1;
        int start = -1;
        int maxLen = 0;

        // Binary search for the length of the longest duplicated substring
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int foundStart = search(mid);
            if (foundStart != -1) {
                // Found a duplicate of length mid, try for a longer one
                maxLen = mid;
                start = foundStart;
                low = mid + 1;
            } else {
                // No duplicate of length mid, try for a shorter one
                high = mid - 1;
            }
        }

        if (start == -1) {
            return "";
        }
        return s.substring(start, start + maxLen);
    }
    
    public static void main(String[] args) {
        Task173 solver = new Task173();
        
        String[] testCases = {
            "banana",
            "abcd",
            "ababa",
            "zzzzzzzzzz",
            "mississippi"
        };
        
        String[] expected = {
            "ana",
            "",
            "aba",
            "zzzzzzzzz",
            "issi"
        };

        for (int i = 0; i < testCases.length; i++) {
            String s = testCases[i];
            String result = solver.longestDupSubstring(s);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: s = \"" + s + "\"");
            System.out.println("Output: \"" + result + "\"");
            System.out.println("Expected: \"" + expected[i] + "\"");
            System.out.println();
        }
    }
}