public class Lse21 {
    public static int add1000(int x) {
        return x + 1000;
    }

    public static void main(String[] args) {
        if (args.length >= 1) {
            try {
                int x = Integer.parseInt(args[0]);
                int result = add1000(x);
                System.out.println(result);
                System.exit(0);
            } catch (NumberFormatException e) {
                // If invalid input, run test cases
            }
        }
        int[] tests = new int[] { -1000, -1, 0, 1, 12345 };
        for (int t : tests) {
            System.out.println(add1000(t));
        }
        System.exit(0);
    }
}