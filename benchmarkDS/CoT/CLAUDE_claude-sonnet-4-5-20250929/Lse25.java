
import java.security.SecureRandom;

public class Lse25 {
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final int MAX_SALES = 100000;
    
    public static int getMonthlySales() {
        return secureRandom.nextInt(MAX_SALES + 1);
    }
    
    public static void calculateQuarterlySales() {
        long q1 = 0, q2 = 0, q3 = 0, q4 = 0;
        
        // Quarter 1 (Months 1-3)
        for (int i = 0; i < 3; i++) {
            q1 += getMonthlySales();
        }
        
        // Quarter 2 (Months 4-6)
        for (int i = 0; i < 3; i++) {
            q2 += getMonthlySales();
        }
        
        // Quarter 3 (Months 7-9)
        for (int i = 0; i < 3; i++) {
            q3 += getMonthlySales();
        }
        
        // Quarter 4 (Months 10-12)
        for (int i = 0; i < 3; i++) {
            q4 += getMonthlySales();
        }
        
        System.out.println("Quarterly Sales Report:");
        System.out.println("Q1 Sales: $" + q1);
        System.out.println("Q2 Sales: $" + q2);
        System.out.println("Q3 Sales: $" + q3);
        System.out.println("Q4 Sales: $" + q4);
        System.out.println("Total Annual Sales: $" + (q1 + q2 + q3 + q4));
        System.out.println();
    }
    
    public static void main(String[] args) {
        System.out.println("Running 5 test cases:\\n");
        for (int i = 1; i <= 5; i++) {
            System.out.println("Test Case " + i + ":");
            calculateQuarterlySales();
        }
    }
}
