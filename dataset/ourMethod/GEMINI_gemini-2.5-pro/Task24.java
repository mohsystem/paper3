public class Task24 {

    /**
     * Validates an ATM PIN.
     * A valid PIN must be a string containing exactly 4 or 6 digits.
     *
     * @param pin The string to validate.
     * @return true if the PIN is valid, false otherwise.
     */
    public static boolean validatePin(String pin) {
        // Rule #1: Ensure all input is validated.
        // The regex checks both length (4 or 6) and content (only digits).
        // It's a concise and safe way to validate the format.
        if (pin == null) {
            return false;
        }
        return pin.matches("\\d{4}|\\d{6}");
    }

    public static void main(String[] args) {
        String[] testCases = {"1234", "123456", "12345", "a234", "123", "", "-1234", "1.234", "0000"};
        System.out.println("Running Java test cases...");
        for (int i = 0; i < testCases.length; i++) {
            if (i >= 5) break; // As per prompt, limit to 5 test cases in output.
            String pin = testCases[i];
            System.out.println("PIN: \"" + pin + "\" -> " + validatePin(pin));
        }
    }
}