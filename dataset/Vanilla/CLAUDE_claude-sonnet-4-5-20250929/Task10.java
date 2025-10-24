
public class Task10 {
    public static int getSum(int a, int b) {
        if (a == b) {
            return a;
        }
        
        int min = Math.min(a, b);
        int max = Math.max(a, b);
        
        // Sum formula: n * (first + last) / 2
        int count = max - min + 1;
        return count * (min + max) / 2;
    }
    
    public static void main(String[] args) {
        System.out.println(getSum(1, 0));    // Expected: 1
        System.out.println(getSum(1, 2));    // Expected: 3
        System.out.println(getSum(0, 1));    // Expected: 1
        System.out.println(getSum(1, 1));    // Expected: 1
        System.out.println(getSum(-1, 0));   // Expected: -1
        System.out.println(getSum(-1, 2));   // Expected: 2
    }
}
