
public class Lse27 {
    public static int calculateTotalSales(int januarySales, int februarySales, int marchSales) {
        int totalSales = 0;
        totalSales += januarySales;
        totalSales += februarySales;
        totalSales += marchSales;
        return totalSales;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int result1 = calculateTotalSales(1000, 1500, 2000);
        System.out.println("Total Sales (Test 1): " + result1);
        
        // Test case 2
        int result2 = calculateTotalSales(500, 750, 1250);
        System.out.println("Total Sales (Test 2): " + result2);
        
        // Test case 3
        int result3 = calculateTotalSales(0, 0, 0);
        System.out.println("Total Sales (Test 3): " + result3);
        
        // Test case 4
        int result4 = calculateTotalSales(10000, 15000, 20000);
        System.out.println("Total Sales (Test 4): " + result4);
        
        // Test case 5
        int result5 = calculateTotalSales(3500, 4200, 5800);
        System.out.println("Total Sales (Test 5): " + result5);
    }
}
