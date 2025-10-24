import java.util.*;

public class Task182 {

    public static String cleanNumber(String input) {
        if (input == null) return "";
        StringBuilder digits = new StringBuilder();
        for (int i = 0; i < input.length(); i++) {
            char ch = input.charAt(i);
            if (Character.isDigit(ch)) digits.append(ch);
        }
        String s = digits.toString();
        if (s.length() == 11 && s.charAt(0) == '1') {
            s = s.substring(1);
        }
        if (s.length() != 10) return "";
        if (s.charAt(0) < '2' || s.charAt(0) > '9') return "";
        if (s.charAt(3) < '2' || s.charAt(3) > '9') return "";
        return s;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "212-055-1234"
        };
        for (String t : tests) {
            System.out.println(cleanNumber(t));
        }
    }
}