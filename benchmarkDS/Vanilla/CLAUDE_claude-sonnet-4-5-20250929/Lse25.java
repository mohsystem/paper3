
import java.util.Random;

public class Lse25 {
    private static Random random = new Random();
    
    public static int getMonthlySales() {
        return random.nextInt(100001);
    }
    
    public static void calculateQuarterlySales() {
        int[] monthlySales = new int[12];
        
        // Get monthly sales for all 12 months
        for (int i = 0; i < 12; i++) {
            monthlySales[i] = getMonthlySales();
        }
        
        // Calculate quarterly sales
        int q1 = monthlySales[0] + monthlySales[1] + monthlySales[2];
        int q2 = monthlySales[3] + monthlySales[4] + monthlySales[5];
        int q3 = monthlySales[6] + monthlySales[7] + monthlySales[8];
        int q4 = monthlySales[9] + monthlySales[10] + monthlySales[11];
        
        // Print results
        System.out.println("Quarter 1 Sales: $" + q1);
        System.out.println("Quarter 2 Sales: $" + q2);
        System.out.println("Quarter 3 Sales: $" + q3);
        System.out.println("Quarter 4 Sales: $" + q4);
        System.out.println("Total Annual Sales: $" + (q1 + q2 + q3 + q4));
        System.out.println();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        calculateQuarterlySales();
        
        System.out.println("Test Case 2:");
        calculateQuarterlySales();
        
        System.out.println("Test Case 3:");
        calculateQuarterlySales();
        
        System.out.println("Test Case 4:");
        calculateQuarterlySales();
        
        System.out.println("Test Case 5:");
        calculateQuarterlySales();
    }
}
