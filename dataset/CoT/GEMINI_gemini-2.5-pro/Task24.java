public class Task24 {

    /**
     * Validates an ATM PIN.
     * A valid PIN must be exactly 4 or 6 digits long and contain only digits.
     *
     * @param pin The string to validate.
     * @return true if the PIN is valid, false otherwise.
     */
    public static boolean validatePin(String pin) {
        // A simple and safe regex to check for exactly 4 or 6 digits.
        // \d{4} matches exactly four digits.
        // \d{6} matches exactly six digits.
        // | is the OR operator.
        // The pattern as a whole checks if the entire string matches one of these two conditions.
        if (pin == null) {
            return false;
        }
        return pin.matches("\\d{4}|\\d{6}");
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("Testing Java version:");
        // Test Case 1: Valid 4-digit PIN
        System.out.println("\"1234\"   -->  " + validatePin("1234"));
        // Test Case 2: Invalid length
        System.out.println("\"12345\"  -->  " + validatePin("12345"));
        // Test Case 3: Invalid character
        System.out.println("\"a234\"   -->  " + validatePin("a234"));
        // Test Case 4: Valid 6-digit PIN
        System.out.println("\"451352\" -->  " + validatePin("451352"));
        // Test Case 5: Invalid characters and length
        System.out.println("\"-1234\"  -->  " + validatePin("-1234"));
    }
}