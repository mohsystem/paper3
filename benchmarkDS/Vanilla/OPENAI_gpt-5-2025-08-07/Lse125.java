public class Lse125 {
    public static String trimTrailing(String s) {
        if (s == null) return null;
        return s.replaceAll("\\s+$", "");
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            System.out.println(trimTrailing(args[0]));
        } else {
            String[] tests = {
                "hello   ",
                "test\t\t",
                "line\n",
                " mix  \t\n",
                "no-trim"
            };
            for (String t : tests) {
                System.out.println(trimTrailing(t));
            }
        }
    }
}