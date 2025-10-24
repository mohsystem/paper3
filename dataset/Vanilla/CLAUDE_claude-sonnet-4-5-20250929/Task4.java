
import java.util.HashSet;
import java.util.Set;

public class Task4 {
    public static boolean isIsogram(String str) {
        if (str == null || str.isEmpty()) {
            return true;
        }
        
        String lowerStr = str.toLowerCase();
        Set<Character> charSet = new HashSet<>();
        
        for (char c : lowerStr.toCharArray()) {
            if (charSet.contains(c)) {
                return false;
            }
            charSet.add(c);
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: 'Dermatoglyphics' -> " + isIsogram("Dermatoglyphics"));
        System.out.println("Test 2: 'aba' -> " + isIsogram("aba"));
        System.out.println("Test 3: 'moOse' -> " + isIsogram("moOse"));
        System.out.println("Test 4: '' -> " + isIsogram(""));
        System.out.println("Test 5: 'isogram' -> " + isIsogram("isogram"));
    }
}
