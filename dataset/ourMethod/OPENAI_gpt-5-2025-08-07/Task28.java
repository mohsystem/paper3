import java.util.Arrays;

public class Task28 {

    public static String likesText(String[] names) {
        // Validate and sanitize input
        if (names == null) {
            names = new String[0];
        }
        int n = names.length;

        String n1 = n > 0 ? sanitizeName(names[0]) : "";
        String n2 = n > 1 ? sanitizeName(names[1]) : "";
        String n3 = n > 2 ? sanitizeName(names[2]) : "";

        switch (n) {
            case 0:
                return "no one likes this";
            case 1:
                return n1 + " likes this";
            case 2:
                return n1 + " and " + n2 + " like this";
            case 3:
                return n1 + ", " + n2 + " and " + n3 + " like this";
            default:
                int others = n - 2;
                return n1 + ", " + n2 + " and " + others + " others like this";
        }
    }

    private static String sanitizeName(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder(100);
        int count = 0;
        for (int i = 0; i < input.length() && count < 100; i++) {
            char ch = input.charAt(i);
            if (!Character.isISOControl(ch)) {
                sb.append(ch);
                count++;
            }
        }
        return sb.toString().trim();
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] tests = new String[][]{
                new String[]{},
                new String[]{"Peter"},
                new String[]{"Jacob", "Alex"},
                new String[]{"Max", "John", "Mark"},
                new String[]{"Alex", "Jacob", "Mark", "Max"}
        };

        for (String[] test : tests) {
            System.out.println(Arrays.toString(test) + " --> " + likesText(test));
        }
    }
}