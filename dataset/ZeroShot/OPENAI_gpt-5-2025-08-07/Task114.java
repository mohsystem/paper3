import java.util.Locale;

public class Task114 {

    public static String copyString(String s) {
        if (s == null) return null;
        return new String(s);
    }

    public static String concatStrings(String a, String b) {
        if (a == null || b == null) return null;
        long totalLen = (long) a.length() + (long) b.length();
        if (totalLen > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Resulting string too large");
        }
        StringBuilder sb = new StringBuilder((int) totalLen);
        sb.append(a);
        sb.append(b);
        return sb.toString();
    }

    public static String reverseString(String s) {
        if (s == null) return null;
        char[] arr = s.toCharArray();
        int i = 0, j = arr.length - 1;
        while (i < j) {
            char tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
        return new String(arr);
    }

    public static String toUpperCaseSafe(String s) {
        if (s == null) return null;
        return s.toUpperCase(Locale.ROOT);
    }

    public static String replaceAllSafe(String s, String target, String replacement) {
        if (s == null || target == null || replacement == null) return null;
        if (target.isEmpty()) {
            // Avoid ambiguous behavior on empty target; return a copy
            return new String(s);
        }
        return s.replace(target, replacement);
    }

    // Optional utility
    public static boolean isPalindrome(String s) {
        if (s == null) return false;
        int i = 0, j = s.length() - 1;
        while (i < j) {
            char ci = s.charAt(i);
            char cj = s.charAt(j);
            if (!Character.isLetterOrDigit(ci)) { i++; continue; }
            if (!Character.isLetterOrDigit(cj)) { j--; continue; }
            if (Character.toLowerCase(ci) != Character.toLowerCase(cj)) return false;
            i++; j--;
        }
        return true;
    }

    public static void main(String[] args) {
        // 5 test cases
        String t1 = copyString("Hello, World!");
        System.out.println("Copy: " + t1);

        String t2 = concatStrings("Hello ", "World");
        System.out.println("Concat: " + t2);

        String t3 = reverseString("abcdefg");
        System.out.println("Reverse: " + t3);

        String t4 = toUpperCaseSafe("SecuRe123!");
        System.out.println("Upper: " + t4);

        String t5 = replaceAllSafe("the cat sat on the mat", "at", "oodle");
        System.out.println("Replace: " + t5);
    }
}