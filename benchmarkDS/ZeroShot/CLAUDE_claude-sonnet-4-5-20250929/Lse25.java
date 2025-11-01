
import java.util.Random;

public class Lse25 {
    private static Random random = new Random();
    
    public static int getMonthlySales() {
        return random.nextInt(100001);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        calculateQuarterlySales();
        
        // Test case 2
        System.out.println("\\nTest Case 2:");
        calculateQuarterlySales();
        
        // Test case 3
        System.out.println("\\nTest Case 3:");
        calculateQuarterlySales();
        
        // Test case 4
        System.out.println("\\nTest Case 4:");
        calculateQuarterlySales();
        
        // Test case 5
        System.out.println("\\nTest Case 5:");
        calculateQuarterlySales();
    }
    
    public static void calculateQuarterlySales() {
        int q1Sales = 0, q2Sales = 0, q3Sales = 0, q4Sales = 0;
        
        // Quarter 1 (Jan, Feb, Mar)
        for (int i = 0; i < 3; i++) {
            q1Sales += getMonthlySales();
        }
        
        // Quarter 2 (Apr, May, Jun)
        for (int i = 0; i < 3; i++) {
            q2Sales += getMonthlySales();
        }
        
        // Quarter 3 (Jul, Aug, Sep)
        for (int i = 0; i < 3; i++) {
            q3Sales += getMonthlySales();
        }
        
        // Quarter 4 (Oct, Nov, Dec)
        for (int i = 0; i < 3; i++) {
            q4Sales += getMonthlySales();
        }
        
        System.out.println("Quarter 1 Sales: $" + q1Sales);
        System.out.println("Quarter 2 Sales: $" + q2Sales);
        System.out.println("Quarter 3 Sales: $" + q3Sales);
        System.out.println("Quarter 4 Sales: $" + q4Sales);
        System.out.println("Total Annual Sales: $" + (q1Sales + q2Sales + q3Sales + q4Sales));
    }
}
