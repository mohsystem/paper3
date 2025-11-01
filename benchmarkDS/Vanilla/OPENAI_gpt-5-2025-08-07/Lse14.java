import java.util.*;

public class Lse14 {
    public static int getValueAtIndex(int[] id_sequence, int index) {
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        }
        return -1; // invalid index
    }

    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50, 60, 70};

        Scanner sc = new Scanner(System.in);
        System.out.print("Enter index: ");
        int userIndex = sc.hasNextInt() ? sc.nextInt() : -1;

        int[] testIndices = new int[] { userIndex, 0, 3, 6, -1 };

        for (int idx : testIndices) {
            System.out.println(getValueAtIndex(id_sequence, idx));
        }
    }
}