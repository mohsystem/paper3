// Task182 - Java implementation
// Chain-of-Through process:
// 1) Problem understanding: Clean NANP phone numbers by removing punctuation and optional country code (1 or +1), ensuring resulting number is 10 digits with NXX NXX-XXXX constraints (area and exchange codes start with 2-9).
// 2) Security requirements: Validate inputs, avoid exceptions on null, no external input execution, and prevent overflows.
// 3) Secure coding generation: Use Character checks, StringBuilder, and validations.
// 4) Code review: Validate length, country code, and NXX constraints.
// 5) Secure code output: Return a clean 10-digit string or empty string if invalid.

public class Task182 {
    public static String clean(String input) {
        if (input == null) return "";
        StringBuilder digits = new StringBuilder();
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (Character.isDigit(c)) {
                digits.append(c);
            }
        }
        int len = digits.length();
        if (len == 11) {
            if (digits.charAt(0) != '1') return "";
            digits.deleteCharAt(0);
            len = 10;
        }
        if (len != 10) return "";
        char areaFirst = digits.charAt(0);
        char exchangeFirst = digits.charAt(3);
        if (areaFirst < '2' || areaFirst > '9') return "";
        if (exchangeFirst < '2' || exchangeFirst > '9') return "";
        return digits.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "1 123 456 7890"
        };
        for (String t : tests) {
            System.out.println(clean(t));
        }
    }
}