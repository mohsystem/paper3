
import java.util.HashSet;
import java.util.Set;

public class Task4 {
    public static boolean isIsogram(String input) {
        if (input == null) {
            return false;
        }
        
        if (input.isEmpty()) {
            return true;
        }
        
        // Validate input contains only letters
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (!Character.isLetter(c)) {
                return false;
            }
        }
        
        // Convert to lowercase and check for duplicates
        String lowercase = input.toLowerCase();
        Set<Character> seen = new HashSet<>();
        
        for (int i = 0; i < lowercase.length(); i++) {
            char c = lowercase.charAt(i);
            if (seen.contains(c)) {
                return false;
            }
            seen.add(c);
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1
        String test1 = "Dermatoglyphics";
        System.out.println(test1 + " --> " + isIsogram(test1));
        
        // Test case 2
        String test2 = "aba";
        System.out.println(test2 + " --> " + isIsogram(test2));
        
        // Test case 3
        String test3 = "moOse";
        System.out.println(test3 + " --> " + isIsogram(test3));
        
        // Test case 4
        String test4 = "";
        System.out.println("\\"\\" --> " + isIsogram(test4));
        
        // Test case 5
        String test5 = "isogram";
        System.out.println(test5 + " --> " + isIsogram(test5));
    }
}
