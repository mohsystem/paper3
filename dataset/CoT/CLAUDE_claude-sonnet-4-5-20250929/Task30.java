
public class Task30 {
    public static String longest(String s1, String s2) {
        if (s1 == null || s2 == null) {
            return "";
        }
        
        boolean[] seen = new boolean[26];
        
        for (int i = 0; i < s1.length(); i++) {
            char c = s1.charAt(i);
            if (c >= 'a' && c <= 'z') {
                seen[c - 'a'] = true;
            }
        }
        
        for (int i = 0; i < s2.length(); i++) {
            char c = s2.charAt(i);
            if (c >= 'a' && c <= 'z') {
                seen[c - 'a'] = true;
            }
        }
        
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < 26; i++) {
            if (seen[i]) {
                result.append((char)('a' + i));
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        String a1 = "xyaabbbccccdefww";
        String b1 = "xxxxyyyyabklmopq";
        System.out.println("Test 1: " + longest(a1, b1));
        
        // Test case 2
        String a2 = "abcdefghijklmnopqrstuvwxyz";
        System.out.println("Test 2: " + longest(a2, a2));
        
        // Test case 3
        String a3 = "abc";
        String b3 = "xyz";
        System.out.println("Test 3: " + longest(a3, b3));
        
        // Test case 4
        String a4 = "zzz";
        String b4 = "aaa";
        System.out.println("Test 4: " + longest(a4, b4));
        
        // Test case 5
        String a5 = "";
        String b5 = "abc";
        System.out.println("Test 5: " + longest(a5, b5));
    }
}
