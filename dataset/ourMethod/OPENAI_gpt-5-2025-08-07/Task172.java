import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.Objects;

public class Task172 {

    private static final long BASE1 = 11400714819323198485L; // random odd 64-bit
    private static final long BASE2 = 14029467366897019727L; // another random odd 64-bit

    private static long[] pow1;
    private static long[] pow2;
    private static long[] pref1;
    private static long[] pref2;

    private static final class Key {
        final long h1;
        final long h2;
        final int len;

        Key(long h1, long h2, int len) {
            this.h1 = h1;
            this.h2 = h2;
            this.len = len;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (!(o instanceof Key)) return false;
            Key key = (Key) o;
            return h1 == key.h1 && h2 == key.h2 && len == key.len;
        }

        @Override
        public int hashCode() {
            long x = h1 * 0x9E3779B97F4A7C15L ^ (h2 + 0xC2B2AE3D27D4EB4FL) ^ ((long) len * 0x165667B19E3779F9L);
            x ^= (x >>> 33);
            x *= 0xff51afd7ed558ccdL;
            x ^= (x >>> 33);
            x *= 0xc4ceb9fe1a85ec53L;
            x ^= (x >>> 33);
            return (int) (x ^ (x >>> 32));
        }
    }

    private static void validate(String text) {
        if (text == null) throw new IllegalArgumentException("text is null");
        int n = text.length();
        if (n < 1 || n > 2000) throw new IllegalArgumentException("length out of range");
        for (int i = 0; i < n; i++) {
            char c = text.charAt(i);
            if (c < 'a' || c > 'z') throw new IllegalArgumentException("invalid character");
        }
    }

    private static void buildHashes(String s) {
        int n = s.length();
        pow1 = new long[n + 1];
        pow2 = new long[n + 1];
        pref1 = new long[n + 1];
        pref2 = new long[n + 1];
        pow1[0] = 1L;
        pow2[0] = 1L;
        for (int i = 1; i <= n; i++) {
            pow1[i] = pow1[i - 1] * BASE1;
            pow2[i] = pow2[i - 1] * BASE2;
        }
        pref1[0] = 0L;
        pref2[0] = 0L;
        for (int i = 0; i < n; i++) {
            int v = (s.charAt(i) - 'a' + 1);
            pref1[i + 1] = pref1[i] * BASE1 + v;
            pref2[i + 1] = pref2[i] * BASE2 + v;
        }
    }

    private static long getHash1(int l, int r) {
        // [l, r)
        return pref1[r] - pref1[l] * pow1[r - l];
    }

    private static long getHash2(int l, int r) {
        // [l, r)
        return pref2[r] - pref2[l] * pow2[r - l];
    }

    public static int countSquareSubstringsDistinct(String text) {
        validate(text);
        int n = text.length();
        if (n < 2) return 0;

        buildHashes(text);

        HashSet<Key> set = new HashSet<>();
        for (int i = 0; i < n; i++) {
            for (int len = 2; i + len <= n; len += 2) {
                int mid = i + (len >>> 1);
                long a1 = getHash1(i, mid);
                long b1 = getHash1(mid, i + len);
                if (a1 != b1) continue;
                long a2 = getHash2(i, mid);
                long b2 = getHash2(mid, i + len);
                if (a2 != b2) continue;
                long h1 = getHash1(i, i + len);
                long h2 = getHash2(i, i + len);
                set.add(new Key(h1, h2, len));
            }
        }
        return set.size();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "abcabcabc",          // 3
                "leetcodeleetcode",   // 2
                "aaaaaa",             // 3 -> "aa","aaaa","aaaaaa"
                "ababab",             // 2 -> "abab","baba"
                "a"                   // 0
        };
        for (String t : tests) {
            int res = countSquareSubstringsDistinct(t);
            System.out.println("Input: " + t + " -> " + res);
        }
    }
}