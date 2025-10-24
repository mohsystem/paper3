
public class Task24 {
    public static boolean validatePin(String pin) {
        if (pin == null) {
            return false;
        }
        
        int length = pin.length();
        if (length != 4 && length != 6) {
            return false;
        }
        
        for (int i = 0; i < length; i++) {
            char c = pin.charAt(i);
            if (c < '0' || c > '9') {
                return false;
            }
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(validatePin("1234"));   // true
        System.out.println(validatePin("123456")); // true
        System.out.println(validatePin("12345"));  // false
        System.out.println(validatePin("a234"));   // false
        System.out.println(validatePin(""));       // false
    }
}
