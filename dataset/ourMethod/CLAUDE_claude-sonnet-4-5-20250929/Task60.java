
import java.util.ArrayList;
import java.util.List;

public class Task60 {
    public static String expandedForm(int num) {
        // Input validation: ensure num is greater than 0
        if (num <= 0) {
            throw new IllegalArgumentException("Number must be greater than 0");
        }
        
        // Prevent integer overflow by checking bounds
        if (num > Integer.MAX_VALUE || num < 0) {
            throw new IllegalArgumentException("Number out of valid range");
        }
        
        String numStr = String.valueOf(num);
        List<String> parts = new ArrayList<>();
        int length = numStr.length();
        
        for (int i = 0; i < length; i++) {
            char digit = numStr.charAt(i);
            if (digit != '0') {
                int powerOfTen = length - i - 1;
                long expandedValue = (digit - '0') * (long)Math.pow(10, powerOfTen);
                parts.add(String.valueOf(expandedValue));
            }
        }
        
        return String.join(" + ", parts);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(expandedForm(12));
        
        // Test case 2
        System.out.println(expandedForm(42));
        
        // Test case 3
        System.out.println(expandedForm(70304));
        
        // Test case 4
        System.out.println(expandedForm(9));
        
        // Test case 5
        System.out.println(expandedForm(123456));
    }
}
