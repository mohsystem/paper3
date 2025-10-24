import java.util.Arrays;

public class Task28 {
    public static String likes(String[] names) {
        if (names == null || names.length == 0) {
            return "no one likes this";
        }
        int n = names.length;
        String a = safe(names[0]);
        switch (n) {
            case 1:
                return a + " likes this";
            case 2: {
                String b = safe(names[1]);
                return a + " and " + b + " like this";
            }
            case 3: {
                String b = safe(names[1]);
                String c = safe(names[2]);
                return a + ", " + b + " and " + c + " like this";
            }
            default: {
                String b = safe(names[1]);
                return a + ", " + b + " and " + (n - 2) + " others like this";
            }
        }
    }

    private static String safe(String s) {
        return (s == null) ? "null" : s;
    }

    public static void main(String[] args) {
        String[][] tests = new String[][] {
            new String[] {},
            new String[] {"Peter"},
            new String[] {"Jacob", "Alex"},
            new String[] {"Max", "John", "Mark"},
            new String[] {"Alex", "Jacob", "Mark", "Max"}
        };
        for (String[] t : tests) {
            System.out.println(Arrays.toString(t) + " --> " + likes(t));
        }
    }
}