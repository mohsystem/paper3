
public class Lse26 {
    public static double getMonthlySales(int month) {
        // Simulated monthly sales data
        double[] sales = {15000.50, 22000.75, 18500.25, 20000.00, 25000.50, 
                         19000.75, 21000.25, 23000.00, 17000.50, 24000.75, 
                         26000.25, 28000.00};
        if (month >= 0 && month < sales.length) {
            return sales[month];
        }
        return 0.0;
    }
    
    public static double calculateFirstQuarterSales() {
        double sum = 0.0;
        for (int i = 0; i < 3; i++) {
            sum += getMonthlySales(i);
        }
        return sum;
    }
    
    public static void main(String[] args) {
        // Test case 1: Calculate first quarter sales
        System.out.println("Test Case 1 - First Quarter Sales: $" + calculateFirstQuarterSales());
        
        // Test case 2: Get individual monthly sales
        System.out.println("Test Case 2 - Month 0 Sales: $" + getMonthlySales(0));
        
        // Test case 3: Get individual monthly sales
        System.out.println("Test Case 3 - Month 1 Sales: $" + getMonthlySales(1));
        
        // Test case 4: Get individual monthly sales
        System.out.println("Test Case 4 - Month 2 Sales: $" + getMonthlySales(2));
        
        // Test case 5: Verify sum calculation
        double manualSum = getMonthlySales(0) + getMonthlySales(1) + getMonthlySales(2);
        System.out.println("Test Case 5 - Manual Sum Verification: $" + manualSum);
    }
}
