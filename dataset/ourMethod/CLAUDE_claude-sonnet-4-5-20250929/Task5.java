
public class Task5 {
    public static int persistence(int num) {
        if (num < 0) {
            throw new IllegalArgumentException("Input must be a positive number");
        }
        
        if (num < 10) {
            return 0;
        }
        
        int count = 0;
        long current = num;
        
        while (current >= 10) {
            long product = 1;
            while (current > 0) {
                product *= current % 10;
                current /= 10;
            }
            current = product;
            count++;
            
            if (count > 100) {
                throw new IllegalStateException("Exceeded maximum iterations");
            }
        }
        
        return count;
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1: persistence(39) = " + persistence(39));
        System.out.println("Test 2: persistence(999) = " + persistence(999));
        System.out.println("Test 3: persistence(4) = " + persistence(4));
        System.out.println("Test 4: persistence(25) = " + persistence(25));
        System.out.println("Test 5: persistence(0) = " + persistence(0));
    }
}
