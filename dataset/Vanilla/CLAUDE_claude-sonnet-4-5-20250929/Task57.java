
public class Task57 {
    public static long factorial(int n) {
        if (n < 0) {
            throw new IllegalArgumentException("Factorial is not defined for negative numbers");
        }
        if (n == 0 || n == 1) {
            return 1;
        }
        long result = 1;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Factorial of 0: " + factorial(0));
        
        // Test case 2
        System.out.println("Factorial of 1: " + factorial(1));
        
        // Test case 3
        System.out.println("Factorial of 5: " + factorial(5));
        
        // Test case 4
        System.out.println("Factorial of 10: " + factorial(10));
        
        // Test case 5
        System.out.println("Factorial of 15: " + factorial(15));
    }
}
