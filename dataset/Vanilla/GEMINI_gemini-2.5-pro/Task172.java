import java.util.HashSet;
import java.util.Set;

public class Task172 {

    public int countDistinct(String text) {
        int n = text.length();
        if (n <= 1) {
            return 0;
        }

        long p1 = 31, m1 = 1_000_000_007L;
        long p2 = 37, m2 = 1_000_000_009L;

        long[] p1_powers = new long[n + 1];
        long[] p2_powers = new long[n + 1];
        p1_powers[0] = 1;
        p2_powers[0] = 1;
        for (int i = 1; i <= n; i++) {
            p1_powers[i] = (p1_powers[i - 1] * p1) % m1;
            p2_powers[i] = (p2_powers[i - 1] * p2) % m2;
        }

        long[] h1 = new long[n + 1];
        long[] h2 = new long[n + 1];
        for (int i = 0; i < n; i++) {
            h1[i + 1] = (h1[i] * p1 + (text.charAt(i) - 'a' + 1)) % m1;
            h2[i + 1] = (h2[i] * p2 + (text.charAt(i) - 'a' + 1)) % m2;
        }

        Set<Long> foundHashes = new HashSet<>();
        
        // len is the length of 'a'
        for (int len = 1; len * 2 <= n; len++) {
            // i is the starting index of 'a+a'
            for (int i = 0; i <= n - 2 * len; i++) {
                int mid_idx = i + len;
                
                // Hash of the first half
                long hash1_a = (h1[mid_idx] - (h1[i] * p1_powers[len]) % m1 + m1) % m1;
                long hash2_a = (h2[mid_idx] - (h2[i] * p2_powers[len]) % m2 + m2) % m2;

                // Hash of the second half
                long hash1_b = (h1[i + 2 * len] - (h1[mid_idx] * p1_powers[len]) % m1 + m1) % m1;
                long hash2_b = (h2[i + 2 * len] - (h2[mid_idx] * p2_powers[len]) % m2 + m2) % m2;

                if (hash1_a == hash1_b && hash2_a == hash2_b) {
                    // Hashes match, add the hash of the whole substring 'a+a' to the set
                    long total_hash1 = (h1[i + 2 * len] - (h1[i] * p1_powers[2 * len]) % m1 + m1) % m1;
                    long total_hash2 = (h2[i + 2 * len] - (h2[i] * p2_powers[2 * len]) % m2 + m2) % m2;
                    foundHashes.add(total_hash1 * m2 + total_hash2);
                }
            }
        }
        return foundHashes.size();
    }

    public static void main(String[] args) {
        Task172 solution = new Task172();
        
        String text1 = "abcabcabc";
        System.out.println("Input: " + text1 + ", Output: " + solution.countDistinct(text1));

        String text2 = "leetcodeleetcode";
        System.out.println("Input: " + text2 + ", Output: " + solution.countDistinct(text2));

        String text3 = "aaaaa";
        System.out.println("Input: " + text3 + ", Output: " + solution.countDistinct(text3));
        
        String text4 = "ababa";
        System.out.println("Input: " + text4 + ", Output: " + solution.countDistinct(text4));

        String text5 = "zzzzzzzzzz";
        System.out.println("Input: " + text5 + ", Output: " + solution.countDistinct(text5));
    }
}