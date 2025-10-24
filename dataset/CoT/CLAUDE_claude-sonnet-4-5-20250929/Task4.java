
import java.util.HashSet;
import java.util.Set;

public class Task4 {
    public static boolean isIsogram(String str) {
        if (str == null) {
            return true;
        }
        
        String lowerStr = str.toLowerCase();
        Set<Character> seenChars = new HashSet<>();
        
        for (int i = 0; i < lowerStr.length(); i++) {
            char c = lowerStr.charAt(i);
            if (Character.isLetter(c)) {
                if (seenChars.contains(c)) {
                    return false;
                }
                seenChars.add(c);
            }
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: 'Dermatoglyphics' -> " + isIsogram("Dermatoglyphics"));
        
        // Test case 2
        System.out.println("Test 2: 'aba' -> " + isIsogram("aba"));
        
        // Test case 3
        System.out.println("Test 3: 'moOse' -> " + isIsogram("moOse"));
        
        // Test case 4
        System.out.println("Test 4: '' -> " + isIsogram(""));
        
        // Test case 5
        System.out.println("Test 5: 'isogram' -> " + isIsogram("isogram"));
    }
}
