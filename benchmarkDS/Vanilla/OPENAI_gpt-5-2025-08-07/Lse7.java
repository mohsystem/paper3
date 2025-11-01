public class Lse7 {
    public static int strToInt(String s) {
        if (s == null) return 0;
        int i = 0, n = s.length();
        while (i < n && Character.isWhitespace(s.charAt(i))) i++;
        int sign = 1;
        if (i < n && (s.charAt(i) == '+' || s.charAt(i) == '-')) {
            if (s.charAt(i) == '-') sign = -1;
            i++;
        }
        long val = 0;
        while (i < n && Character.isDigit(s.charAt(i))) {
            val = val * 10 + (s.charAt(i) - '0');
            i++;
        }
        return (int) (sign * val);
    }

    public static void printAtIndex(String[] args, int index) {
        if (index >= 0 && index < args.length) {
            System.out.println(args[index]);
        }
    }

    public static void main(String[] args) {
        if (args.length > 1) {
            int idx = strToInt(args[1]);
            printAtIndex(args, idx);
            return;
        }

        // 5 test cases (run when no CLI index provided)
        String[] t1 = new String[]{"Lse7", "2", "zero", "one", "two"};
        printAtIndex(t1, strToInt(t1[1]));

        String[] t2 = new String[]{"Lse7", "3", "red", "green", "blue", "yellow"};
        printAtIndex(t2, strToInt(t2[1]));

        String[] t3 = new String[]{"Lse7", "1", "hello"};
        printAtIndex(t3, strToInt(t3[1]));

        String[] t4 = new String[]{"Lse7", "4", "a", "b", "c", "d", "e"};
        printAtIndex(t4, strToInt(t4[1]));

        String[] t5 = new String[]{"Lse7", "0", "first"};
        printAtIndex(t5, strToInt(t5[1]));
    }
}