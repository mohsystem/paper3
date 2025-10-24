import java.util.*;

public class Task28 {
    public static String likes(String[] names) {
        int n = names == null ? 0 : names.length;
        if (n == 0) return "no one likes this";
        if (n == 1) return names[0] + " likes this";
        if (n == 2) return names[0] + " and " + names[1] + " like this";
        if (n == 3) return names[0] + ", " + names[1] + " and " + names[2] + " like this";
        return names[0] + ", " + names[1] + " and " + (n - 2) + " others like this";
    }

    public static void main(String[] args) {
        System.out.println(likes(new String[]{}));
        System.out.println(likes(new String[]{"Peter"}));
        System.out.println(likes(new String[]{"Jacob", "Alex"}));
        System.out.println(likes(new String[]{"Max", "John", "Mark"}));
        System.out.println(likes(new String[]{"Alex", "Jacob", "Mark", "Max", "Lily"}));
    }
}