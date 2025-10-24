
import java.util.ArrayList;
import java.util.List;

public class Task22 {
    public static List<Integer> filterList(List<Object> list) {
        if (list == null) {
            throw new IllegalArgumentException("Input list cannot be null");
        }
        
        List<Integer> result = new ArrayList<>();
        
        for (Object element : list) {
            if (element == null) {
                continue;
            }
            
            if (element instanceof Integer) {
                int value = (Integer) element;
                if (value >= 0) {
                    result.add(value);
                }
            } else if (element instanceof Long || element instanceof Short || element instanceof Byte) {
                long value = ((Number) element).longValue();
                if (value >= 0 && value <= Integer.MAX_VALUE) {
                    result.add((int) value);
                }
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1: " + filterList(List.of(1, 2, "a", "b")));
        System.out.println("Test 2: " + filterList(List.of(1, "a", "b", 0, 15)));
        System.out.println("Test 3: " + filterList(List.of(1, 2, "a", "b", "aasf", "1", "123", 123)));
        
        List<Object> test4 = new ArrayList<>();
        test4.add(5);
        test4.add("test");
        test4.add(10);
        test4.add(null);
        test4.add(20);
        System.out.println("Test 4: " + filterList(test4));
        
        System.out.println("Test 5: " + filterList(List.of(100, 200, "xyz", 300)));
    }
}
