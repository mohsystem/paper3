public class Lse5 {
    public static int getElement(int[] arr, int index) {
        return arr[index];
    }

    public static void main(String[] args) {
        int[] arr = new int[] {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};

        if (args.length > 0) {
            try {
                int idx = Integer.parseInt(args[0]);
                if (idx >= 0 && idx < arr.length) {
                    int val = getElement(arr, idx);
                    System.out.println(val);
                } else {
                    System.out.println("Index out of bounds");
                }
            } catch (NumberFormatException e) {
                System.out.println("Invalid integer argument");
            }
            return;
        }

        int[] tests = {0, 1, 4, 7, 9};
        for (int idx : tests) {
            int val = getElement(arr, idx);
            System.out.println(val);
        }
    }
}