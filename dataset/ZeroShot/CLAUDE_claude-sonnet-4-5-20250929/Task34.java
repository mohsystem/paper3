
public class Task34 {
    public static boolean isNarcissistic(int number) {
        String numStr = String.valueOf(number);
        int numDigits = numStr.length();
        int sum = 0;
        
        for (char digit : numStr.toCharArray()) {
            int digitValue = Character.getNumericValue(digit);
            sum += Math.pow(digitValue, numDigits);
        }
        
        return sum == number;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - Input: 153, Output: " + isNarcissistic(153));
        System.out.println("Test 2 - Input: 1652, Output: " + isNarcissistic(1652));
        System.out.println("Test 3 - Input: 9474, Output: " + isNarcissistic(9474));
        System.out.println("Test 4 - Input: 9475, Output: " + isNarcissistic(9475));
        System.out.println("Test 5 - Input: 9, Output: " + isNarcissistic(9));
    }
}
