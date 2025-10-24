
import java.util.*;

public class Task173 {
    private static final long MOD = (1L << 61) - 1;
    private static final long BASE = 26;
    
    public static String longestDupSubstring(String s) {
        if (s == null || s.length() < 2 || s.length() > 30000) {
            return "";
        }
        
        // Validate input: only lowercase English letters
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < 'a' || c > 'z') {
                return "";
            }
        }
        
        int left = 1;
        int right = s.length() - 1;
        String result = "";
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            String dup = search(s, mid);
            if (dup != null && !dup.isEmpty()) {
                result = dup;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return result;
    }
    
    private static String search(String s, int len) {
        if (len <= 0 || len > s.length()) {
            return null;
        }
        
        long hash = 0;
        long power = 1;
        
        for (int i = 0; i < len; i++) {
            hash = modMul(hash, BASE);
            hash = (hash + (s.charAt(i) - 'a')) % MOD;
            if (i < len - 1) {
                power = modMul(power, BASE);
            }
        }
        
        Map<Long, List<Integer>> seen = new HashMap<>();
        seen.computeIfAbsent(hash, k -> new ArrayList<>()).add(0);
        
        for (int i = 1; i <= s.length() - len; i++) {
            hash = (hash + MOD - modMul(power, (s.charAt(i - 1) - 'a'))) % MOD;
            hash = modMul(hash, BASE);
            hash = (hash + (s.charAt(i + len - 1) - 'a')) % MOD;
            
            if (seen.containsKey(hash)) {
                String current = s.substring(i, i + len);
                for (int pos : seen.get(hash)) {
                    if (s.substring(pos, pos + len).equals(current)) {
                        return current;
                    }
                }
            }
            seen.computeIfAbsent(hash, k -> new ArrayList<>()).add(i);
        }
        
        return null;
    }
    
    private static long modMul(long a, long b) {
        return (a * b) % MOD;
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1: " + longestDupSubstring("banana"));
        System.out.println("Test 2: " + longestDupSubstring("abcd"));
        System.out.println("Test 3: " + longestDupSubstring("aa"));
        System.out.println("Test 4: " + longestDupSubstring("aaaaa"));
        System.out.println("Test 5: " + longestDupSubstring("abcdefghijklmn"));
    }
}
