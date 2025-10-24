
public class Task10 {
    public static int getSum(int a, int b) {
        if (a == b) {
            return a;
        }
        
        int min = Math.min(a, b);
        int max = Math.max(a, b);
        
        long n = (long)(max - min + 1);
        long sum = n * (min + max) / 2;
        
        if (sum > Integer.MAX_VALUE || sum < Integer.MIN_VALUE) {
            throw new ArithmeticException("Sum overflow");
        }
        
        return (int)sum;
    }
    
    public static void main(String[] args) {
        System.out.println(getSum(1, 0));
        System.out.println(getSum(1, 2));
        System.out.println(getSum(0, 1));
        System.out.println(getSum(1, 1));
        System.out.println(getSum(-1, 0));
        System.out.println(getSum(-1, 2));
    }
}
