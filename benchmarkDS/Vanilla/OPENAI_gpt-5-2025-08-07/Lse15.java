import java.util.*;

public class Lse15 {
    public static int getValueAtIndex(int[] id_sequence, int index) {
        return id_sequence[index];
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int[] id_sequence = new int[]{11, 22, 33, 44, 55, 66, 77, 88, 99};

        System.out.print("Enter index: ");
        if (sc.hasNextInt()) {
            int idx = sc.nextInt();
            try {
                int val = getValueAtIndex(id_sequence, idx);
                System.out.println("Value at index " + idx + ": " + val);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println("Invalid index: " + idx);
            }
        } else {
            System.out.println("Invalid input.");
            if (sc.hasNextLine()) sc.nextLine();
        }

        int[] testIndices = new int[]{0, 3, id_sequence.length - 1, -1, 100};
        for (int i = 0; i < testIndices.length; i++) {
            int idx = testIndices[i];
            try {
                int val = getValueAtIndex(id_sequence, idx);
                System.out.println("Test " + (i + 1) + " -> index " + idx + " value: " + val);
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println("Test " + (i + 1) + " -> index " + idx + " invalid");
            }
        }
        sc.close();
    }
}