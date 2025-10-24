
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
        
        // Validate the cleaned number
        if (digitsOnly.length() != 10) {
            return null;
        }
        
        // Validate area code (first digit must be 2-9)
        char areaCodeFirst = digitsOnly.charAt(0);
        if (areaCodeFirst < '2' || areaCodeFirst > '9') {
            return null;
        }
        
        // Validate exchange code (first digit must be 2-9)
        char exchangeFirst = digitsOnly.charAt(3);
        if (exchangeFirst < '2' || exchangeFirst > '9') {
            return null;
        }
        
        return digitsOnly;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + cleanPhoneNumber("+1 (613)-995-0253"));
        System.out.println("Test 2: " + cleanPhoneNumber("613-995-0253"));
        System.out.println("Test 3: " + cleanPhoneNumber("1 613 995 0253"));
        System.out.println("Test 4: " + cleanPhoneNumber("613.995.0253"));
        System.out.println("Test 5: " + cleanPhoneNumber("(234) 567-8901"));
    }
}
