
public class Task24 {
    public static boolean validatePin(String pin) {
        // Null and empty check
        if (pin == null || pin.isEmpty()) {
            return false;
        }
        
        // Length validation - must be exactly 4 or 6
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
        // Test cases
        System.out.println("Test 1 - '1234': " + validatePin("1234"));      // true
        System.out.println("Test 2 - '12345': " + validatePin("12345"));    // false
        System.out.println("Test 3 - 'a234': " + validatePin("a234"));      // false
        System.out.println("Test 4 - '123456': " + validatePin("123456"));  // true
        System.out.println("Test 5 - '': " + validatePin(""));              // false
    }
}
