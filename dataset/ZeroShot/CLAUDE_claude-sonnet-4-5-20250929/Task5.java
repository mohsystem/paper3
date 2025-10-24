
public class Task5 {
    public static int persistence(int num) {
        if (num < 10) {
            return 0;
        }
        
        int count = 0;
        
        while (num >= 10) {
            int product = 1;
            while (num > 0) {
                product *= num % 10;
                num /= 10;
            }
            num = product;
            count++;
        }
        
        return count;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: persistence(39) = " + persistence(39) + " (Expected: 3)");
        System.out.println("Test 2: persistence(999) = " + persistence(999) + " (Expected: 4)");
        System.out.println("Test 3: persistence(4) = " + persistence(4) + " (Expected: 0)");
        System.out.println("Test 4: persistence(25) = " + persistence(25) + " (Expected: 2)");
        System.out.println("Test 5: persistence(77) = " + persistence(77) + " (Expected: 4)");
    }
}
