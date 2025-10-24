
public class Task142 {
    public static int gcd(int a, int b) {
        a = Math.abs(a);
        b = Math.abs(b);
        
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("GCD of 48 and 18: " + gcd(48, 18));
        
        // Test case 2
        System.out.println("GCD of 100 and 50: " + gcd(100, 50));
        
        // Test case 3
        System.out.println("GCD of 17 and 19: " + gcd(17, 19));
        
        // Test case 4
        System.out.println("GCD of 270 and 192: " + gcd(270, 192));
        
        // Test case 5
        System.out.println("GCD of 1071 and 462: " + gcd(1071, 462));
    }
}
