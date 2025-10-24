import java.util.*;

public class Task58 {
    public static boolean XO(String s) {
        int x = 0, o = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = Character.toLowerCase(s.charAt(i));
            if (c == 'x') x++;
            else if (c == 'o') o++;
        }
        return x == o;
    }

    public static void main(String[] args) {
        String[] tests = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
        for (String t : tests) {
            System.out.println("XO(\"" + t + "\") => " + XO(t));
        }
    }
}