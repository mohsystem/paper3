import java.util.*;

public class Task173 {
    public static String longestDupSubstring(String s) {
        if (s == null || s.length() < 2) return "";
        int n = s.length();
        int[] sa = buildSuffixArray(s);
        int[] lcp = buildLCP(s, sa);
        int maxL = 0, pos = -1;
        for (int i = 0; i < n - 1; i++) {
            if (lcp[i] > maxL) {
                maxL = lcp[i];
                pos = sa[i];
            }
        }
        return maxL == 0 ? "" : s.substring(pos, pos + maxL);
    }

    private static int[] buildSuffixArray(String s) {
        int n = s.length();
        int[] sa = new int[n];
        int[] rnk = new int[n];
        int[] tmp = new int[n];

        for (int i = 0; i < n; i++) {
            sa[i] = i;
            rnk[i] = s.charAt(i) - 'a';
        }

        int[] key1 = new int[n];
        int[] key2 = new int[n];
        int[] out = new int[n];

        for (int k = 1; k < n; k <<= 1) {
            int maxRank = 0;
            for (int v : rnk) if (v > maxRank) maxRank = v;

            for (int i = 0; i < n; i++) key2[i] = (i + k < n ? rnk[i + k] + 1 : 0);
            countingSort(sa, key2, maxRank + 2, out);

            for (int i = 0; i < n; i++) key1[i] = rnk[i] + 1;
            countingSort(out, key1, maxRank + 2, sa);

            tmp[sa[0]] = 0;
            int classes = 1;
            for (int i = 1; i < n; i++) {
                int cur = sa[i], prev = sa[i - 1];
                if (rnk[cur] != rnk[prev] || key2[cur] != key2[prev]) classes++;
                tmp[cur] = classes - 1;
            }
            System.arraycopy(tmp, 0, rnk, 0, n);
            if (classes == n) break;
        }
        return sa;
    }

    private static void countingSort(int[] saIn, int[] key, int maxKey, int[] saOut) {
        int n = saIn.length;
        int[] cnt = new int[maxKey + 1];
        Arrays.fill(cnt, 0);
        for (int i = 0; i < n; i++) cnt[key[saIn[i]]]++;
        int sum = 0;
        for (int i = 0; i <= maxKey; i++) {
            int t = cnt[i];
            cnt[i] = sum;
            sum += t;
        }
        for (int i = 0; i < n; i++) {
            int idx = saIn[i];
            int k = key[idx];
            saOut[cnt[k]++] = idx;
        }
    }

    private static int[] buildLCP(String s, int[] sa) {
        int n = s.length();
        int[] rank = new int[n];
        for (int i = 0; i < n; i++) rank[sa[i]] = i;
        int[] lcp = new int[n - 1];
        int k = 0;
        for (int i = 0; i < n; i++) {
            int r = rank[i];
            if (r == n - 1) {
                k = 0;
                continue;
            }
            int j = sa[r + 1];
            while (i + k < n && j + k < n && s.charAt(i + k) == s.charAt(j + k)) k++;
            lcp[r] = k;
            if (k > 0) k--;
        }
        return lcp;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "banana",
            "abcd",
            "aaaaa",
            "abcabca",
            "mississippi"
        };
        for (String t : tests) {
            System.out.println("Input: " + t + " -> Output: " + longestDupSubstring(t));
        }
    }
}