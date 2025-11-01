public class Lse125 {
    public static String trimTrailing(String s) {
        if (s == null) return null;
        int i = s.length();
        while (i > 0 && Character.isWhitespace(s.charAt(i - 1))) {
            i--;
        }
        return s.substring(0, i);
    }

    public static void main(String[] args) {
        if (args != null && args.length > 0) {
            System.out.println(trimTrailing(args[0]));
        } else {
            String[] tests = new String[] {
                "Hello World   ",
                "TrailingTabs\t\t\t",
                "LineEnd\n\n",
                "  LeadingShouldStay   ",
                "    "
            };
            for (String t : tests) {
                System.out.println(trimTrailing(t));
            }
        }
    }
}