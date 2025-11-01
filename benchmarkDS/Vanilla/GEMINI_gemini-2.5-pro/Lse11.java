import java.util.Scanner;

public class Lse11 {

    public static int getValueFromArray(int[] arr, int size, int index) {
        return arr[index];
    }

    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50};
        int size = id_sequence.length;

        // 5 Test cases using assertions
        assert getValueFromArray(id_sequence, size, 0) == 10;
        assert getValueFromArray(id_sequence, size, 1) == 20;
        assert getValueFromArray(id_sequence, size, 2) == 30;
        assert getValueFromArray(id_sequence, size, 3) == 40;
        assert getValueFromArray(id_sequence, size, 4) == 50;

        Scanner scanner = new Scanner(System.in);
        int index = scanner.nextInt();
        scanner.close();

        int value = getValueFromArray(id_sequence, size, index);

        System.exit(value);
    }
}