
public class Task34 {
    public static boolean isNarcissistic(int number) {
        // Input validation - ensure positive integer
        if (number < 0) {
            return false;
        }
        
        // Convert to string to get digits and count
        String numStr = String.valueOf(number);
        int numDigits = numStr.length();
        
        // Calculate sum of digits raised to power of digit count
        int sum = 0;
        for (int i = 0; i < numDigits; i++) {
            int digit = Character.getNumericValue(numStr.charAt(i));
            // Use Math.pow and cast to int to avoid overflow for reasonable inputs
            sum += (int) Math.pow(digit, numDigits);
        }
        
        return sum == number;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - 153: " + isNarcissistic(153)); // true
        System.out.println("Test 2 - 1652: " + isNarcissistic(1652)); // false
        System.out.println("Test 3 - 9: " + isNarcissistic(9)); // true
        System.out.println("Test 4 - 9474: " + isNarcissistic(9474)); // true
        System.out.println("Test 5 - 370: " + isNarcissistic(370)); // true
    }
}
