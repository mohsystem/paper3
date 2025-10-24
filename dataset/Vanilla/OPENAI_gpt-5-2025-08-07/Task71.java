public class Task71 {
    // Converts a string to Integer; returns null if invalid
    public static Integer convertToInt(String s) {
        if (s == null) return null;
        try {
            return Integer.parseInt(s.trim());
        } catch (NumberFormatException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            for (String a : args) {
                Integer r = convertToInt(a);
                if (r != null) {
                    System.out.println("Input: \"" + a + "\" -> " + r);
                } else {
                    System.out.println("Input: \"" + a + "\" -> invalid");
                }
            }
        } else {
            String[] tests = {"123", "  42 ", "-7", "abc", "2147483648"};
            for (String t : tests) {
                Integer r = convertToInt(t);
                if (r != null) {
                    System.out.println("Input: \"" + t + "\" -> " + r);
                } else {
                    System.out.println("Input: \"" + t + "\" -> invalid");
                }
            }
        }
    }
}