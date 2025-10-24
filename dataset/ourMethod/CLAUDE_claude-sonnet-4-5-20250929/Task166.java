
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Task166 {
    public static List<Integer> posNegSort(List<Integer> arr) {
        if (arr == null) {
            return new ArrayList<>();
        }
        
        // Create a copy to avoid modifying the input
        List<Integer> result = new ArrayList<>(arr);
        
        // Extract positive numbers and sort them
        List<Integer> positives = new ArrayList<>();
        for (Integer num : result) {
            if (num > 0) {
                positives.add(num);
            }
        }
        Collections.sort(positives);
        
        // Replace positive numbers in result with sorted ones
        int posIndex = 0;
        for (int i = 0; i < result.size(); i++) {
            if (result.get(i) > 0) {
                result.set(i, positives.get(posIndex));
                posIndex++;
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        List<Integer> test1 = new ArrayList<>();
        test1.add(6); test1.add(3); test1.add(-2); test1.add(5); test1.add(-8); test1.add(2); test1.add(-2);
        System.out.println(posNegSort(test1));
        
        // Test case 2
        List<Integer> test2 = new ArrayList<>();
        test2.add(6); test2.add(5); test2.add(4); test2.add(-1); test2.add(3); test2.add(2); test2.add(-1); test2.add(1);
        System.out.println(posNegSort(test2));
        
        // Test case 3
        List<Integer> test3 = new ArrayList<>();
        test3.add(-5); test3.add(-5); test3.add(-5); test3.add(-5); test3.add(7); test3.add(-5);
        System.out.println(posNegSort(test3));
        
        // Test case 4
        List<Integer> test4 = new ArrayList<>();
        System.out.println(posNegSort(test4));
        
        // Test case 5
        List<Integer> test5 = new ArrayList<>();
        test5.add(-1); test5.add(-2); test5.add(-3);
        System.out.println(posNegSort(test5));
    }
}
