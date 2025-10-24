import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

public class Task173 {

    private long modulus = (1L << 31) - 1;
    private long base = 26;

    public String longestDupSubstring(String s) {
        int n = s.length();
        int low = 1, high = n;
        int start = -1;
        int len = 0;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (mid == 0) {
                low = mid + 1;
                continue;
            }
            int foundStart = search(mid, n, s);
            if (foundStart != -1) {
                len = mid;
                start = foundStart;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        return start != -1 ? s.substring(start, start + len) : "";
    }
    
    private long power(long b, long exp) {
        long res = 1;
        b %= modulus;
        while (exp > 0) {
            if (exp % 2 == 1) res = (res * b) % modulus;
            b = (b * b) % modulus;
            exp /= 2;
        }
        return res;
    }

    private int search(int L, int n, String s) {
        if (L == 0) return 0;
        
        long h = power(base, L - 1);

        long currentHash = 0;
        for (int i = 0; i < L; i++) {
            currentHash = (currentHash * base + (s.charAt(i) - 'a')) % modulus;
        }

        HashMap<Long, ArrayList<Integer>> seen = new HashMap<>();
        seen.put(currentHash, new ArrayList<>(Collections.singletonList(0)));

        for (int i = 1; i <= n - L; i++) {
            long termToRemove = ((long)(s.charAt(i - 1) - 'a') * h) % modulus;
            currentHash = (currentHash - termToRemove + modulus) % modulus;
            currentHash = (currentHash * base) % modulus;
            currentHash = (currentHash + (s.charAt(i + L - 1) - 'a')) % modulus;

            if (seen.containsKey(currentHash)) {
                for (int startIdx : seen.get(currentHash)) {
                    if (s.substring(startIdx, startIdx + L).equals(s.substring(i, i + L))) {
                        return i;
                    }
                }
            }
            seen.computeIfAbsent(currentHash, k -> new ArrayList<>()).add(i);
        }
        return -1;
    }
    
    public static void main(String[] args) {
        Task173 solver = new Task173();
        String[] testCases = {
            "banana",
            "abcd",
            "ababab",
            "aaaaa",
            "aabaaabaaaba"
        };
        for (String s : testCases) {
            System.out.println("Input: \"" + s + "\"");
            System.out.println("Output: \"" + solver.longestDupSubstring(s) + "\"");
            System.out.println();
        }
    }
}