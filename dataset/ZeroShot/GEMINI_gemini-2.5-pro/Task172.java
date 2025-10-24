import java.util.HashSet;
import java.util.Set;

public class Task172 {

    /**
     * Helper method to calculate the hash of a substring text[i..j].
     * Uses precomputed prefix hashes and powers of P.
     * @param h array of prefix hashes
     * @param p_pow array of powers of P
     * @param M the modulus
     * @param i start index of substring
     * @param j end index of substring
     * @return the hash value
     */
    private long getHash(long[] h, long[] p_pow, long M, int i, int j) {
        int len = j - i + 1;
        long term = (h[i] * p_pow[len]) % M;
        long raw_hash = h[j + 1] - term;
        // Ensure the result is non-negative
        return (raw_hash % M + M) % M;
    }

    /**
     * Calculates the number of distinct non-empty substrings that can be written
     * as the concatenation of some string with itself.
     * @param text The input string.
     * @return The number of such distinct substrings.
     */
    public int distinctEchoSubstrings(String text) {
        long P1 = 31, M1 = 1_000_000_007;
        long P2 = 37, M2 = 1_000_000_009;
        int n = text.length();

        long[] p_pow1 = new long[n + 1];
        long[] h1 = new long[n + 1];
        long[] p_pow2 = new long[n + 1];
        long[] h2 = new long[n + 1];

        p_pow1[0] = 1;
        p_pow2[0] = 1;
        h1[0] = 0;
        h2[0] = 0;

        for (int i = 0; i < n; i++) {
            p_pow1[i + 1] = (p_pow1[i] * P1) % M1;
            h1[i + 1] = (h1[i] * P1 + (text.charAt(i) - 'a' + 1)) % M1;
            p_pow2[i + 1] = (p_pow2[i] * P2) % M2;
            h2[i + 1] = (h2[i] * P2 + (text.charAt(i) - 'a' + 1)) % M2;
        }

        Set<String> foundHashes = new HashSet<>();
        for (int l = 1; l <= n / 2; l++) { // l is the length of the repeating part 'a'
            for (int i = 0; i <= n - 2 * l; i++) { // i is the start of 'a+a'
                int j = i + l; // j is the start of the second 'a'
                
                // Compare text[i...i+l-1] and text[j...j+l-1] using double hashing
                long h1_a = getHash(h1, p_pow1, M1, i, i + l - 1);
                long h1_b = getHash(h1, p_pow1, M1, j, j + l - 1);

                if (h1_a == h1_b) {
                    long h2_a = getHash(h2, p_pow2, M2, i, i + l - 1);
                    long h2_b = getHash(h2, p_pow2, M2, j, j + l - 1);
                    if (h2_a == h2_b) {
                        // Strings are very likely equal. Add the hash of the 'a+a' substring.
                        long hash1_aa = getHash(h1, p_pow1, M1, i, i + 2 * l - 1);
                        long hash2_aa = getHash(h2, p_pow2, M2, i, i + 2 * l - 1);
                        foundHashes.add(hash1_aa + ":" + hash2_aa);
                    }
                }
            }
        }
        return foundHashes.size();
    }

    public static void main(String[] args) {
        Task172 solution = new Task172();
        
        String[] testCases = {
            "abcabcabc",       // Expected: 3
            "leetcodeleetcode",// Expected: 2
            "a",               // Expected: 0
            "aaaaa",           // Expected: 2
            "abacaba"          // Expected: 0
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String text = testCases[i];
            int result = solution.distinctEchoSubstrings(text);
            System.out.println("Input: text = \"" + text + "\"");
            System.out.println("Output: " + result);
            System.out.println();
        }
    }
}