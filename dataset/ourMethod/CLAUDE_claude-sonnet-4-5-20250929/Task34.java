
public class Task34 {
    public static boolean isNarcissistic(int value) {
        // Validate input is positive
        if (value < 0) {
            throw new IllegalArgumentException("Input must be positive");
        }
        
        // Convert to string to get digits safely
        String numStr = Integer.toString(value);
        int numDigits = numStr.length();
        
        // Validate string length to prevent overflow
        if (numDigits > 10) {
            return false;
        }
        
        long sum = 0;
        for (int i = 0; i < numDigits; i++) {
            int digit = Character.digit(numStr.charAt(i), 10);
            if (digit < 0 || digit > 9) {
                throw new IllegalArgumentException("Invalid digit");
            }
            // Use long to prevent overflow
            long power = 1;
            for (int j = 0; j < numDigits; j++) {
                power *= digit;
                // Check for overflow
                if (power < 0) {
                    return false;
                }
            }
            sum += power;
            // Check for overflow
            if (sum < 0) {
                return false;
            }
        }
        
        return sum == (long) value;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(isNarcissistic(7) == true);      // 7^1 = 7
        System.out.println(isNarcissistic(153) == true);    // 1^3 + 5^3 + 3^3 = 153
        System.out.println(isNarcissistic(1652) == false);  // Not narcissistic
        System.out.println(isNarcissistic(9474) == true);   // 9^4 + 4^4 + 7^4 + 4^4 = 9474
        System.out.println(isNarcissistic(100) == false);   // Not narcissistic
    }
}
