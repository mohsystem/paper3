
import java.util.Set;
import java.util.TreeSet;
import java.nio.charset.StandardCharsets;

public class Task30 {
    public static String longest(String s1, String s2) {
        if (s1 == null || s2 == null) {
            throw new IllegalArgumentException("Input strings must not be null");
        }
        
        if (s1.length() > 10000 || s2.length() > 10000) {
            throw new IllegalArgumentException("Input strings exceed maximum length");
        }
        
        Set<Character> distinctChars = new TreeSet<>();
        
        for (int i = 0; i < s1.length(); i++) {
            char c = s1.charAt(i);
            if (c < 'a' || c > 'z') {
                throw new IllegalArgumentException("Input must contain only lowercase letters a-z");
            }
            distinctChars.add(c);
        }
        
        for (int i = 0; i < s2.length(); i++) {
            char c = s2.charAt(i);
            if (c < 'a' || c > 'z') {
                throw new IllegalArgumentException("Input must contain only lowercase letters a-z");
            }
            distinctChars.add(c);
        }
        
        StringBuilder result = new StringBuilder(distinctChars.size());
        for (Character c : distinctChars) {
            result.append(c);
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        System.out.println(longest("xyaabbbccccdefww", "xxxxyyyyabklmopq"));
        System.out.println(longest("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"));
        System.out.println(longest("abc", "xyz"));
        System.out.println(longest("aaa", "bbb"));
        System.out.println(longest("", "abc"));
    }
}
