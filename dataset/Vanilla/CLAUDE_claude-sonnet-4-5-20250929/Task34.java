
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
        System.out.println("Test 1: isNarcissistic(153) = " + isNarcissistic(153)); // true
        System.out.println("Test 2: isNarcissistic(1652) = " + isNarcissistic(1652)); // false
        System.out.println("Test 3: isNarcissistic(9) = " + isNarcissistic(9)); // true
        System.out.println("Test 4: isNarcissistic(9474) = " + isNarcissistic(9474)); // true
        System.out.println("Test 5: isNarcissistic(1634) = " + isNarcissistic(1634)); // true
    }
}
