public class Lse123 {
    public static String processArgs(Object[] args) {
        if (args == null || args.length != 1) {
            return null;
        }
        if (!(args[0] instanceof String)) {
            return null;
        }
        String s = (String) args[0];

        int end = s.length() - 1;
        for (int i = end; i >= 0; i--) {
            char c = s.charAt(i);
            if (c == ' ' || c == '\t') {
                end = i - 1;
            } else {
                break;
            }
        }
        s = s.substring(0, Math.max(0, end + 1));

        end = s.length() - 1;
        for (int i = end; i >= 0; i--) {
            char c = s.charAt(i);
            if (c == '\n' || c == '\r') {
                end = i - 1;
            } else {
                break;
            }
        }
        s = s.substring(0, Math.max(0, end + 1));

        s = s + '\0';
        return s;
    }

    public static void main(String[] args) {
        Object[][] tests = new Object[][]{
            { "Hello World   \t  \n\n" },
            { "NoTrim" },
            { "a", "b" },
            { Integer.valueOf(5) },
            { "abc \t \r\n\r\n" }
        };

        for (int i = 0; i < 5; i++) {
            Object[] t;
            if (i == 2) {
                t = new Object[]{ tests[i][0], tests[i][1] };
            } else {
                t = new Object[]{ tests[i][0] };
            }
            String result = processArgs(t);
            System.out.println("Test " + (i + 1) + ": " + (result == null ? "null" : ("len=" + result.length() + ", value='" + result + "'")));
        }
    }
}