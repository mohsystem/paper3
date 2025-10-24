import java.util.Arrays;

public class Task28 {

    public static String likes(String[] names) {
        String[] safeNames = names == null ? new String[0] : names;
        int n = safeNames.length;

        switch (n) {
            case 0:
                return "no one likes this";
            case 1:
                return (safeNames[0] == null ? "" : safeNames[0]) + " likes this";
            case 2:
                return (safeNames[0] == null ? "" : safeNames[0]) + " and " + (safeNames[1] == null ? "" : safeNames[1]) + " like this";
            case 3:
                return (safeNames[0] == null ? "" : safeNames[0]) + ", " + (safeNames[1] == null ? "" : safeNames[1]) + " and " + (safeNames[2] == null ? "" : safeNames[2]) + " like this";
            default:
                int others = n - 2;
                return (safeNames[0] == null ? "" : safeNames[0]) + ", " + (safeNames[1] == null ? "" : safeNames[1]) + " and " + others + " others like this";
        }
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
                {},
                {"Peter"},
                {"Jacob", "Alex"},
                {"Max", "John", "Mark"},
                {"Alex", "Jacob", "Mark", "Max"}
        };

        for (int i = 0; i < tests.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + likes(tests[i]));
        }
    }
}