public class Task60 {
    public static String expandedForm(long n) {
        if (n <= 0) return "";
        char[] digits = Long.toString(n).toCharArray();
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        int len = digits.length;
        for (int i = 0; i < len; i++) {
            char d = digits[i];
            if (d != '0') {
                if (!first) sb.append(" + ");
                sb.append(d);
                int zeros = len - 1 - i;
                for (int z = 0; z < zeros; z++) sb.append('0');
                first = false;
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        long[] tests = {12, 42, 70304, 900000, 10501};
        for (long t : tests) {
            System.out.println(expandedForm(t));
        }
    }
}