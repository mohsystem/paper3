
import java.util.*;

public class Task30 {
    public static String longest(String s1, String s2) {
        if (s1 == null || s2 == null) {
            return "";
        }
        
        Set<Character> uniqueChars = new TreeSet<>();
        
        for (char c : s1.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                uniqueChars.add(c);
            }
        }
        
        for (char c : s2.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                uniqueChars.add(c);
            }
        }
        
        StringBuilder result = new StringBuilder();
        for (char c : uniqueChars) {
            result.append(c);
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
        String b2 = "abcdefghijklmnopqrstuvwxyz";
        System.out.println("Test 2: " + longest(a2, b2));
        
        // Test case 3
        String a3 = "aretheyhere";
        String b3 = "yestheyarehere";
        System.out.println("Test 3: " + longest(a3, b3));
        
        // Test case 4
        String a4 = "loopingisfunbutdangerous";
        String b4 = "lessdangerousthancoding";
        System.out.println("Test 4: " + longest(a4, b4));
        
        // Test case 5
        String a5 = "";
        String b5 = "abc";
        System.out.println("Test 5: " + longest(a5, b5));
    }
}
