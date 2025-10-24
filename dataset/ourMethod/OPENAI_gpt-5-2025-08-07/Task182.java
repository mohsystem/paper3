import java.nio.charset.StandardCharsets;

public class Task182 {
    public static String cleanNumber(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder digits = new StringBuilder();
        byte[] bytes = input.getBytes(StandardCharsets.UTF_8);
        for (byte b : bytes) {
            char c = (char) (b & 0xFF);
            if (c >= '0' && c <= '9') {
                digits.append(c);
            }
        }

        String d = digits.toString();
        if (d.length() == 11 && d.charAt(0) == '1') {
            d = d.substring(1);
        } else if (d.length() != 10) {
            return "";
        }

        if (d.charAt(0) < '2' || d.charAt(0) > '9') return "";
        if (d.charAt(3) < '2' || d.charAt(3) > '9') return "";

        return d;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "1 (123) 456-7890"
        };
        for (String t : tests) {
            String cleaned = cleanNumber(t);
            System.out.println(cleaned);
        }
    }
}