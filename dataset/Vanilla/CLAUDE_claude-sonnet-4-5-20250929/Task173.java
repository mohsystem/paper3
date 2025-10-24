
class Task173 {
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
        long mod = (1L << 32);
        long base = 26;
        long hash = 0;
        long pow = 1;
        
        for (int i = 0; i < len; i++) {
            hash = (hash * base + (s.charAt(i) - 'a')) % mod;
            if (i < len - 1) {
                pow = (pow * base) % mod;
            }
        }
        
        java.util.Set<Long> seen = new java.util.HashSet<>();
        seen.add(hash);
        
        for (int i = len; i < s.length(); i++) {
            hash = (hash * base - (s.charAt(i - len) - 'a') * pow % mod + mod) % mod;
            hash = (hash + (s.charAt(i) - 'a')) % mod;
            
            if (seen.contains(hash)) {
                return s.substring(i - len + 1, i + 1);
            }
            seen.add(hash);
        }
        
        return null;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + longestDupSubstring("banana")); // Expected: "ana"
        System.out.println("Test 2: " + longestDupSubstring("abcd")); // Expected: ""
        System.out.println("Test 3: " + longestDupSubstring("aa")); // Expected: "a"
        System.out.println("Test 4: " + longestDupSubstring("aaaaa")); // Expected: "aaaa"
        System.out.println("Test 5: " + longestDupSubstring("abcabcabc")); // Expected: "abcabc"
    }
}
