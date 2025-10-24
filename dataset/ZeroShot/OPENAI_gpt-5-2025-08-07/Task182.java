import java.util.ArrayList;
import java.util.List;

public class Task182 {
    public static String cleanNumber(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder digits = new StringBuilder(16);
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (Character.isDigit(c)) {
                digits.append(c);
            }
        }
        int len = digits.length();
        if (len == 11) {
            if (digits.charAt(0) != '1') {
                return "";
            }
            // Remove country code
            digits.deleteCharAt(0);
            len = 10;
        }
        if (len != 10) {
            return "";
        }
        char areaFirst = digits.charAt(0);
        char exchFirst = digits.charAt(3);
        if (areaFirst < '2' || areaFirst > '9') {
            return "";
        }
        if (exchFirst < '2' || exchFirst > '9') {
            return "";
        }
        return digits.toString();
    }

    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        tests.add("+1 (613)-995-0253");
        tests.add("613-995-0253");
        tests.add("1 613 995 0253");
        tests.add("613.995.0253");
        tests.add("123-456-7890"); // invalid (area code starts with 1)

        for (String t : tests) {
            System.out.println(cleanNumber(t));
        }
    }
}