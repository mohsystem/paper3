
public class Task60 {
    public static String expandedForm(int num) {
        if (num <= 0) {
            throw new IllegalArgumentException("Number must be greater than 0");
        }
        
        String numStr = String.valueOf(num);
        StringBuilder result = new StringBuilder();
        int length = numStr.length();
        
        for (int i = 0; i < length; i++) {
            char digit = numStr.charAt(i);
            if (digit != '0') {
                if (result.length() > 0) {
                    result.append(" + ");
                }
                result.append(digit);
                for (int j = i + 1; j < length; j++) {
                    result.append('0');
                }
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(expandedForm(12));      // "10 + 2"
        System.out.println(expandedForm(42));      // "40 + 2"
        System.out.println(expandedForm(70304));   // "70000 + 300 + 4"
        System.out.println(expandedForm(9));       // "9"
        System.out.println(expandedForm(100000));  // "100000"
    }
}
