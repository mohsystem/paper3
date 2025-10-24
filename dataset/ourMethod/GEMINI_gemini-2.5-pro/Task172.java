import java.util.HashSet;
import java.util.Set;

public class Task172 {

    /**
     * Returns the number of distinct non-empty substrings of text that can be written
     * as the concatenation of some string with itself.
     *
     * @param text The input string.
     * @return The number of distinct echo substrings.
     */
    public int distinctEchoSubstrings(String text) {
        int n = text.length();
        if (n == 0) {
            return 0;
        }

        // Use double hashing to minimize collisions.
        long p1 = 31, m1 = 1_000_000_007;
        long p2 = 37, m2 = 1_000_000_009;

        long[] p_pow1 = new long[n + 1];
        long[] h1 = new long[n + 1];
        long[] p_pow2 = new long[n + 1];
        long[] h2 = new long[n + 1];

        p_pow1[0] = 1;
        p_pow2[0] = 1;
        h1[0] = 0;
        h2[0] = 0;

        for (int i = 0; i < n; i++) {
            p_pow1[i + 1] = (p_pow1[i] * p1) % m1;
            h1[i + 1] = (h1[i] * p1 + (text.charAt(i) - 'a' + 1)) % m1;
            p_pow2[i + 1] = (p_pow2[i] * p2) % m2;
            h2[i + 1] = (h2[i] * p2 + (text.charAt(i) - 'a' + 1)) % m2;
        }

        Set<Long> distinctHashes = new HashSet<>();
        // len is the length of the full substring 'a+a'
        for (int len = 2; len <= n; len += 2) {
            for (int i = 0; i <= n - len; i++) {
                int half = len / 2;
                int j = i + half;
                int k = i + len;

                // Hash of first half: text[i...j-1]
                long hash1_a = (h1[j] - (h1[i] * p_pow1[half]) % m1 + m1) % m1;
                // Hash of second half: text[j...k-1]
                long hash1_b = (h1[k] - (h1[j] * p_pow1[half]) % m1 + m1) % m1;
                
                if (hash1_a == hash1_b) {
                    long hash2_a = (h2[j] - (h2[i] * p_pow2[half]) % m2 + m2) % m2;
                    long hash2_b = (h2[k] - (h2[j] * p_pow2[half]) % m2 + m2) % m2;
                    
                    if (hash2_a == hash2_b) {
                        // Hashes of halves match, confirming an echo substring.
                        // Add the hash of the full substring text[i...k-1] to the set.
                        long full_hash1 = (h1[k] - (h1[i] * p_pow1[len]) % m1 + m1) % m1;
                        long full_hash2 = (h2[k] - (h2[i] * p_pow2[len]) % m2 + m2) % m2;
                        distinctHashes.add((full_hash1 << 32) | full_hash2);
                    }
                }
            }
        }

        return distinctHashes.size();
    }

    public static void main(String[] args) {
        Task172 sol = new Task172();

        String text1 = "abcabcabc";
        System.out.println("Input: " + text1 + ", Output: " + sol.distinctEchoSubstrings(text1));

        String text2 = "leetcodeleetcode";
        System.out.println("Input: " + text2 + ", Output: " + sol.distinctEchoSubstrings(text2));

        String text3 = "abacaba";
        System.out.println("Input: " + text3 + ", Output: " + sol.distinctEchoSubstrings(text3));

        String text4 = "aaaaa";
        System.out.println("Input: " + text4 + ", Output: " + sol.distinctEchoSubstrings(text4));

        String text5 = "aaaaaaaaaa";
        System.out.println("Input: " + text5 + ", Output: " + sol.distinctEchoSubstrings(text5));
    }
}