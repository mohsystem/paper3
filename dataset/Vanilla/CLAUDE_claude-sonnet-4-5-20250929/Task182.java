
public class Task182 {
    public static String cleanPhoneNumber(String phoneNumber) {
        if (phoneNumber == null) {
            return null;
        }
        
        // Remove all non-digit characters
        String digitsOnly = phoneNumber.replaceAll("[^0-9]", "");
        
        // Check if it starts with country code 1 and has 11 digits
        if (digitsOnly.length() == 11 && digitsOnly.startsWith("1")) {
            digitsOnly = digitsOnly.substring(1);
        }
        
        // Validate the length is 10 digits
        if (digitsOnly.length() != 10) {
            return null;
        }
        
        // Validate area code (first digit must be 2-9)
        if (digitsOnly.charAt(0) < '2' || digitsOnly.charAt(0) > '9') {
            return null;
        }
        
        // Validate exchange code (fourth digit must be 2-9)
        if (digitsOnly.charAt(3) < '2' || digitsOnly.charAt(3) > '9') {
            return null;
        }
        
        return digitsOnly;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + cleanPhoneNumber("+1 (613)-995-0253"));
        
        // Test case 2
        System.out.println("Test 2: " + cleanPhoneNumber("613-995-0253"));
        
        // Test case 3
        System.out.println("Test 3: " + cleanPhoneNumber("1 613 995 0253"));
        
        // Test case 4
        System.out.println("Test 4: " + cleanPhoneNumber("613.995.0253"));
        
        // Test case 5
        System.out.println("Test 5: " + cleanPhoneNumber("(234) 567-8901"));
    }
}
