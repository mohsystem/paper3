
import java.util.Arrays;
import java.util.List;

public class Task9 {
    public static String oddOrEven(List<Integer> array) {
        if (array == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        if (array.isEmpty()) {
            return "even";
        }
        
        long sum = 0;
        for (Integer num : array) {
            if (num == null) {
                throw new IllegalArgumentException("Array elements cannot be null");
            }
            sum += num;
        }
        
        return (sum % 2 == 0) ? "even" : "odd";
    }
    
    public static void main(String[] args) {
        System.out.println(oddOrEven(Arrays.asList(0)));
        System.out.println(oddOrEven(Arrays.asList(0, 1, 4)));
        System.out.println(oddOrEven(Arrays.asList(0, -1, -5)));
        System.out.println(oddOrEven(Arrays.asList()));
        System.out.println(oddOrEven(Arrays.asList(1, 2, 3, 4, 5)));
    }
}
