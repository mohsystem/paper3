import java.util.HashSet;
import java.util.Objects;

public class Task172 {

    // Helper class for storing pairs of hashes, required for the HashSet.
    static class Pair<U, V> {
        public final U first;
        public final V second;

        public Pair(U first, V second) {
            this.first = first;
            this.second = second;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            Pair<?, ?> pair = (Pair<?, ?>) o;
            return Objects.equals(first, pair.first) && Objects.equals(second, pair.second);
        }

        @Override
        public int hashCode() {
            return Objects.hash(first, second);
        }
    }

    public int distinctEchoSubstrings(String text) {
        int n = text.length();
        long p1 = 31, m1 = 1_000_000_007;
        long p2 = 37, m2 = 1_000_000_009;

        long[] p1_powers = new long[n + 1];
        long[] p2_powers = new long[n + 1];
        long[] h1 = new long[n + 1];
        long[] h2 = new long[n + 1];

        p1_powers[0] = 1;
        p2_powers[0] = 1;

        for (int i = 1; i <= n; i++) {
            p1_powers[i] = (p1_powers[i - 1] * p1) % m1;
            p2_powers[i] = (p2_powers[i - 1] * p2) % m2;
        }

        for (int i = 0; i < n; i++) {
            h1[i + 1] = (h1[i] * p1 + (text.charAt(i) - 'a' + 1)) % m1;
            h2[i + 1] = (h2[i] * p2 + (text.charAt(i) - 'a' + 1)) % m2;
        }

        HashSet<Pair<Long, Long>> found = new HashSet<>();
        for (int len = 2; len <= n; len += 2) {
            int half = len / 2;
            for (int i = 0; i <= n - len; i++) {
                int j = i + half;

                // Hash of first half: text[i...j-1]
                long hash1a = (h1[j] - (h1[i] * p1_powers[half]) % m1 + m1) % m1;
                long hash1b = (h2[j] - (h2[i] * p2_powers[half]) % m2 + m2) % m2;
                
                // Hash of second half: text[j...j+half-1]
                long hash2a = (h1[j + half] - (h1[j] * p1_powers[half]) % m1 + m1) % m1;
                long hash2b = (h2[j + half] - (h2[j] * p2_powers[half]) % m2 + m2) % m2;

                if (hash1a == hash2a && hash1b == hash2b) {
                    // Hashes of the two halves match, add the hash of the full substring
                    long fullHashA = (h1[i + len] - (h1[i] * p1_powers[len]) % m1 + m1) % m1;
                    long fullHashB = (h2[i + len] - (h2[i] * p2_powers[len]) % m2 + m2) % m2;
                    found.add(new Pair<>(fullHashA, fullHashB));
                }
            }
        }
        return found.size();
    }

    public static void main(String[] args) {
        Task172 solution = new Task172();
        
        // Test Case 1
        String text1 = "abcabcabc";
        System.out.println("Test Case 1: text = \"" + text1 + "\", Expected: 3, Got: " + solution.distinctEchoSubstrings(text1));

        // Test Case 2
        String text2 = "leetcodeleetcode";
        System.out.println("Test Case 2: text = \"" + text2 + "\", Expected: 2, Got: " + solution.distinctEchoSubstrings(text2));

        // Test Case 3
        String text3 = "aaaaa";
        System.out.println("Test Case 3: text = \"" + text3 + "\", Expected: 2, Got: " + solution.distinctEchoSubstrings(text3));

        // Test Case 4
        String text4 = "ababa";
        System.out.println("Test Case 4: text = \"" + text4 + "\", Expected: 2, Got: " + solution.distinctEchoSubstrings(text4));
        
        // Test Case 5
        String text5 = "a";
        System.out.println("Test Case 5: text = \"" + text5 + "\", Expected: 0, Got: " + solution.distinctEchoSubstrings(text5));
    }
}