
import java.util.*;

public class Task173 {
    public static String longestDupSubstring(String s) {
        int n = s.length();
        int left = 1, right = n;
        String result = "";
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            String dup = search(s, mid);
            
            if (dup != null) {
                result = dup;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return result;
    }
    
    private static String search(String s, int len) {
        long MOD = (1L << 31) - 1;
        long base = 26;
        long hash = 0;
        long power = 1;
        
        Map<Long, List<Integer>> seen = new HashMap<>();
        
        for (int i = 0; i < len; i++) {
            hash = (hash * base + (s.charAt(i) - 'a')) % MOD;
            if (i < len - 1) {
                power = (power * base) % MOD;
            }
        }
        
        seen.computeIfAbsent(hash, k -> new ArrayList<>()).add(0);
        
        for (int i = len; i < s.length(); i++) {
            hash = ((hash - (s.charAt(i - len) - 'a') * power % MOD + MOD) % MOD * base 
                    + (s.charAt(i) - 'a')) % MOD;
            
            if (seen.containsKey(hash)) {
                String current = s.substring(i - len + 1, i + 1);
                for (int idx : seen.get(hash)) {
                    if (s.substring(idx, idx + len).equals(current)) {
                        return current;
                    }
                }
            }
            seen.computeIfAbsent(hash, k -> new ArrayList<>()).add(i - len + 1);
        }
        
        return null;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + longestDupSubstring("banana")); // "ana"
        System.out.println("Test 2: " + longestDupSubstring("abcd")); // ""
        System.out.println("Test 3: " + longestDupSubstring("aa")); // "a"
        System.out.println("Test 4: " + longestDupSubstring("aaaaa")); // "aaaa"
        System.out.println("Test 5: " + longestDupSubstring("abcabcabc")); // "abcabc"
    }
}
