
public class Task20 {
    public static long digPow(long n, int p) {
        // Input validation to prevent security issues
        if (n <= 0 || p <= 0) {
            return -1;
        }
        
        // Convert n to string to extract digits safely
        String nStr = String.valueOf(n);
        long sum = 0;
        
        // Calculate sum of digits raised to consecutive powers
        for (int i = 0; i < nStr.length(); i++) {
            // Validate character is a digit
            char c = nStr.charAt(i);
            if (!Character.isDigit(c)) {
                return -1;
            }
            
            int digit = c - '0';
            int power = p + i;
            
            // Check for potential overflow before calculation
            try {
                long powResult = (long) Math.pow(digit, power);
                // Check if addition would overflow
                if (sum > Long.MAX_VALUE - powResult) {
                    return -1;
                }
                sum += powResult;
            } catch (ArithmeticException e) {
                return -1;
            }
        }
        
        // Check if sum is divisible by n
        if (sum % n == 0) {
            return sum / n;
        }
        
        return -1;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: n=89, p=1 -> " + digPow(89, 1)); // Expected: 1
        
        // Test case 2
        System.out.println("Test 2: n=92, p=1 -> " + digPow(92, 1)); // Expected: -1
        
        // Test case 3
        System.out.println("Test 3: n=695, p=2 -> " + digPow(695, 2)); // Expected: 2
        
        // Test case 4
        System.out.println("Test 4: n=46288, p=3 -> " + digPow(46288, 3)); // Expected: 51
        
        // Test case 5
        System.out.println("Test 5: n=1, p=1 -> " + digPow(1, 1)); // Expected: 1
    }
}
