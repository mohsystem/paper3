
import java.security.SecureRandom;

public class Lse25 {
    private static final SecureRandom secureRandom = new SecureRandom();
    
    /**
     * Returns a random sales value between 0 and 100000 (inclusive).
     * Uses SecureRandom for cryptographically secure random number generation.
     * 
     * @return random sales value in range [0, 100000]
     */
    public static int getMonthlySales() {
        // SecureRandom.nextInt(bound) returns [0, bound), so we use 100001 to include 100000
        return secureRandom.nextInt(100001);
    }
    
    /**
     * Calculates quarterly sales by calling getMonthlySales 12 times
     * and aggregating results into 4 quarters.
     */
    public static void calculateQuarterlySales() {
        // Initialize quarterly sales arrays - explicit initialization for clarity
        int q1 = 0;
        int q2 = 0;
        int q3 = 0;
        int q4 = 0;
        
        // Quarter 1: January, February, March (months 0-2)
        for (int i = 0; i < 3; i++) {
            int sales = getMonthlySales();
            // Check for integer overflow before addition
            if (q1 > Integer.MAX_VALUE - sales) {
                System.err.println("Error: Integer overflow detected in Q1");
                return;
            }
            q1 += sales;
        }
        
        // Quarter 2: April, May, June (months 3-5)
        for (int i = 0; i < 3; i++) {
            int sales = getMonthlySales();
            if (q2 > Integer.MAX_VALUE - sales) {
                System.err.println("Error: Integer overflow detected in Q2");
                return;
            }
            q2 += sales;
        }
        
        // Quarter 3: July, August, September (months 6-8)
        for (int i = 0; i < 3; i++) {
            int sales = getMonthlySales();
            if (q3 > Integer.MAX_VALUE - sales) {
                System.err.println("Error: Integer overflow detected in Q3");
                return;
            }
            q3 += sales;
        }
        
        // Quarter 4: October, November, December (months 9-11)
        for (int i = 0; i < 3; i++) {
            int sales = getMonthlySales();
            if (q4 > Integer.MAX_VALUE - sales) {
                System.err.println("Error: Integer overflow detected in Q4");
                return;
            }
            q4 += sales;
        }
        
        // Print results - using constant format strings for security
        System.out.println("Q1 Sales: " + q1);
        System.out.println("Q2 Sales: " + q2);
        System.out.println("Q3 Sales: " + q3);
        System.out.println("Q4 Sales: " + q4);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        calculateQuarterlySales();
        System.out.println();
        
        // Test case 2
        System.out.println("Test Case 2:");
        calculateQuarterlySales();
        System.out.println();
        
        // Test case 3
        System.out.println("Test Case 3:");
        calculateQuarterlySales();
        System.out.println();
        
        // Test case 4
        System.out.println("Test Case 4:");
        calculateQuarterlySales();
        System.out.println();
        
        // Test case 5
        System.out.println("Test Case 5:");
        calculateQuarterlySales();
    }
}
