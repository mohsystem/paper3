public class Lse20 {
    private static final int OFFSET = 1000;

    public static int addOffset(int value) {
        return value + OFFSET;
    }

    private static void runTests() {
        int[] tests = {0, 1, -1, 42, 999};
        for (int v : tests) {
            System.out.println("Test " + v + " -> " + addOffset(v));
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            try {
                int input = Integer.parseInt(args[0]);
                System.out.println(addOffset(input));
            } catch (NumberFormatException e) {
                System.out.println("Invalid integer input");
            }
        }
        runTests();
    }
}