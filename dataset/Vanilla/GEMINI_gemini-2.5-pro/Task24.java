public class Task24 {

    /**
     * Validates a PIN to be exactly 4 or 6 digits.
     * @param pin The PIN string to validate.
     * @return true if the PIN is valid, false otherwise.
     */
    public static boolean validatePin(String pin) {
        // Use regex to check if the string contains exactly 4 or 6 digits.
        // ^ asserts position at start of the string
        // \\d{4} matches exactly four digits
        // | is the OR operator
        // \\d{6} matches exactly six digits
        // $ asserts position at the end of the string
        return pin.matches("^(\\d{4}|\\d{6})$");
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] testCases = {"1234", "12345", "a234", "123456", "123"};
        
        for (String pin : testCases) {
            System.out.println("\"" + pin + "\"   -->  " + validatePin(pin));
        }
    }
}