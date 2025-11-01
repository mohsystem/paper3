import java.util.Scanner;

public class Lse13 {
    private static final int[] id_sequence = {101, 203, 305, 407, 509, 611, 713, 815, 917, 1020};

    public static int getValueAtIndex(int index) {
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        }
        return -1; // Return -1 for invalid index
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] testIndices = {0, 1, 4, 9, 10};
        System.out.println("Running 5 test cases:");
        for (int idx : testIndices) {
            System.out.println("Index " + idx + " -> " + getValueAtIndex(idx));
        }

        // Prompt user for input
        Scanner sc = new Scanner(System.in);
        System.out.print("Enter index: ");
        if (sc.hasNextInt()) {
            int idx = sc.nextInt();
            int value = getValueAtIndex(idx);
            System.out.println(value);
        } else {
            System.out.println("Invalid input");
        }
        sc.close();
    }
}