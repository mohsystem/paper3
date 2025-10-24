
public class Task5 {
    public static int persistence(int num) {
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
        System.out.println("Input: 39, Output: " + persistence(39));
        System.out.println("Input: 999, Output: " + persistence(999));
        System.out.println("Input: 4, Output: " + persistence(4));
        System.out.println("Input: 25, Output: " + persistence(25));
        System.out.println("Input: 77, Output: " + persistence(77));
    }
}
