
import java.util.Arrays;

public class Task28 {
    public static String whoLikesIt(String[] names) {
        if (names == null) {
            return "no one likes this";
        }
        
        int count = names.length;
        
        if (count == 0) {
            return "no one likes this";
        } else if (count == 1) {
            return sanitizeName(names[0]) + " likes this";
        } else if (count == 2) {
            return sanitizeName(names[0]) + " and " + sanitizeName(names[1]) + " like this";
        } else if (count == 3) {
            return sanitizeName(names[0]) + ", " + sanitizeName(names[1]) + " and " + sanitizeName(names[2]) + " like this";
        } else {
            int others = count - 2;
            return sanitizeName(names[0]) + ", " + sanitizeName(names[1]) + " and " + others + " others like this";
        }
    }
    
    private static String sanitizeName(String name) {
        if (name == null) {
            return "";
        }
        if (name.length() > 100) {
            return name.substring(0, 100);
        }
        return name;
    }
    
    public static void main(String[] args) {
        System.out.println(whoLikesIt(new String[]{}));
        System.out.println(whoLikesIt(new String[]{"Peter"}));
        System.out.println(whoLikesIt(new String[]{"Jacob", "Alex"}));
        System.out.println(whoLikesIt(new String[]{"Max", "John", "Mark"}));
        System.out.println(whoLikesIt(new String[]{"Alex", "Jacob", "Mark", "Max"}));
    }
}
