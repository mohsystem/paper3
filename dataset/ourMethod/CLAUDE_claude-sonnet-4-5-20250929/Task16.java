
import java.nio.charset.StandardCharsets;

public class Task16 {
    public static boolean solution(String str, String ending) {
        // Validate inputs
        if (str == null || ending == null) {
            return false;
        }
        
        // Empty ending always matches
        if (ending.isEmpty()) {
            return true;
        }
        
        // Check if ending is longer than string
        if (ending.length() > str.length()) {
            return false;
        }
        
        // Check if str ends with ending
        return str.endsWith(ending);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(solution("abc", "bc")); // true
        
        // Test case 2
        System.out.println(solution("abc", "d")); // false
        
        // Test case 3
        System.out.println(solution("hello world", "world")); // true
        
        // Test case 4
        System.out.println(solution("test", "")); // true
        
        // Test case 5
        System.out.println(solution("", "test")); // false
    }
}
