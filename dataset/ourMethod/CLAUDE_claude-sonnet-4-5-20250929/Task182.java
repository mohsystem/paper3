
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;

public class Task182 {
    public static String cleanPhoneNumber(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (input.length() > 50) {
            throw new IllegalArgumentException("Input too long");
        }
        
        StringBuilder digits = new StringBuilder();
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (c >= '0' && c <= '9') {
                digits.append(c);
            } else if (c != ' ' && c != '-' && c != '.' && c != '(' && c != ')' && c != '+') {
                throw new IllegalArgumentException("Invalid character in phone number");
            }
        }
        
        String digitString = digits.toString();
        
        if (digitString.length() == 11) {
            if (digitString.charAt(0) != '1') {
                throw new IllegalArgumentException("Invalid country code");
            }
            digitString = digitString.substring(1);
        } else if (digitString.length() != 10) {
            throw new IllegalArgumentException("Invalid phone number length");
        }
        
        if (digitString.length() != 10) {
            throw new IllegalArgumentException("Phone number must be 10 digits");
        }
        
        char areaCode = digitString.charAt(0);
        char exchangeCode = digitString.charAt(3);
        
        if (areaCode < '2' || areaCode > '9') {
            throw new IllegalArgumentException("Area code must start with digit 2-9");
        }
        
        if (exchangeCode < '2' || exchangeCode > '9') {
            throw new IllegalArgumentException("Exchange code must start with digit 2-9");
        }
        
        return digitString;
    }
    
    public static void main(String[] args) {
        List<String> testCases = Arrays.asList(
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "(234) 567-8901"
        );
        
        for (String testCase : testCases) {
            try {
                String result = cleanPhoneNumber(testCase);
                System.out.println("Input: " + testCase + " -> Output: " + result);
            } catch (IllegalArgumentException e) {
                System.out.println("Input: " + testCase + " -> Error: " + e.getMessage());
            }
        }
    }
}
