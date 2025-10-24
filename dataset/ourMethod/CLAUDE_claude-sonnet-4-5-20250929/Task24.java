
import java.nio.charset.StandardCharsets;

public class Task24 {
    public static boolean validatePin(String pin) {
        // Validate input is not null
        if (pin == null) {
            return false;
        }
        
        // Check length is exactly 4 or 6
        int length = pin.length();
        if (length != 4 && length != 6) {
            return false;
        }
        
        // Validate each character is a digit
        for (int i = 0; i < length; i++) {
            char c = pin.charAt(i);
            if (c < '0' || c > '9') {
                return false;
            }
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid 4-digit PIN
        System.out.println(validatePin("1234")); // true
        
        // Test case 2: Invalid 5-digit PIN
        System.out.println(validatePin("12345")); // false
        
        // Test case 3: Invalid PIN with letter
        System.out.println(validatePin("a234")); // false
        
        // Test case 4: Valid 6-digit PIN
        System.out.println(validatePin("123456")); // true
        
        // Test case 5: Null input
        System.out.println(validatePin(null)); // false
    }
}
