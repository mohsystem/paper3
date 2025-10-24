
public class Task182 {
    public static String cleanPhoneNumber(String phoneNumber) {
        if (phoneNumber == null) {
            return null;
        }
        
        // Remove all non-digit characters
        String digitsOnly = phoneNumber.replaceAll("[^0-9]", "");
        
        // Check if empty after cleaning
        if (digitsOnly.isEmpty()) {
            return null;
        }
        
        // Handle country code 1
        if (digitsOnly.length() == 11 && digitsOnly.charAt(0) == '1') {
            digitsOnly = digitsOnly.substring(1);
        }
        
        // Validate length
        if (digitsOnly.length() != 10) {
            return null;
        }
        
        // Validate NANP format: NXX NXX-XXXX
        // First digit of area code (N) must be 2-9
        char firstDigit = digitsOnly.charAt(0);
        if (firstDigit < '2' || firstDigit > '9') {
            return null;
        }
        
        // Fourth digit (first digit of exchange code) must be 2-9
        char fourthDigit = digitsOnly.charAt(3);
        if (fourthDigit < '2' || fourthDigit > '9') {
            return null;
        }
        
        return digitsOnly;
    }
    
    public static void main(String[] args) {
        // Test case 1: Phone number with country code and special characters
        System.out.println("Test 1: " + cleanPhoneNumber("+1 (613)-995-0253"));
        
        // Test case 2: Phone number with dashes
        System.out.println("Test 2: " + cleanPhoneNumber("613-995-0253"));
        
        // Test case 3: Phone number with country code and spaces
        System.out.println("Test 3: " + cleanPhoneNumber("1 613 995 0253"));
        
        // Test case 4: Phone number with dots
        System.out.println("Test 4: " + cleanPhoneNumber("613.995.0253"));
        
        // Test case 5: Phone number with parentheses
        System.out.println("Test 5: " + cleanPhoneNumber("(234) 567-8901"));
    }
}
