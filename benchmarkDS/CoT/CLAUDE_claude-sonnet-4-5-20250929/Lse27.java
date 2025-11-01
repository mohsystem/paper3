
public class Lse27 {
    public static double calculateTotalSales(double month1, double month2, double month3) {
        double totalSales = 0.0;
        totalSales += month1;
        totalSales += month2;
        totalSales += month3;
        return totalSales;
    }
    
    public static void main(String[] args) {
        // Test case 1
        double result1 = calculateTotalSales(1000.50, 2000.75, 1500.25);
        System.out.println("Test 1 - Total Sales: $" + result1);
        
        // Test case 2
        double result2 = calculateTotalSales(5000.00, 3000.00, 4000.00);
        System.out.println("Test 2 - Total Sales: $" + result2);
        
        // Test case 3
        double result3 = calculateTotalSales(750.30, 890.60, 1200.10);
        System.out.println("Test 3 - Total Sales: $" + result3);
        
        // Test case 4
        double result4 = calculateTotalSales(0.00, 0.00, 0.00);
        System.out.println("Test 4 - Total Sales: $" + result4);
        
        // Test case 5
        double result5 = calculateTotalSales(12345.67, 23456.78, 34567.89);
        System.out.println("Test 5 - Total Sales: $" + result5);
    }
}
