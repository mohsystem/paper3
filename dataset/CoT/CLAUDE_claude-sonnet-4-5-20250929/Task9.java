
public class Task9 {
    public static String oddOrEven(int[] array) {
        if (array == null || array.length == 0) {
            array = new int[]{0};
        }
        
        long sum = 0;
        for (int i = 0; i < array.length; i++) {
            sum += array[i];
        }
        
        return (sum % 2 == 0) ? "even" : "odd";
    }
    
    public static void main(String[] args) {
        System.out.println(oddOrEven(new int[]{0}));
        System.out.println(oddOrEven(new int[]{0, 1, 4}));
        System.out.println(oddOrEven(new int[]{0, -1, -5}));
        System.out.println(oddOrEven(new int[]{}));
        System.out.println(oddOrEven(new int[]{1, 2, 3, 4, 5}));
    }
}
