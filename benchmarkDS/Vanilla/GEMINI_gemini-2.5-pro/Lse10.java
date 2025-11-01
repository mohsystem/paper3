public class Lse10 {

    public static int getValueFromArray(int[] arr, int size, int index) {
        // The size parameter is included as per the prompt's requirements.
        // A robust implementation would also check if the index is within bounds.
        return arr[index];
    }

    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int size = id_sequence.length;

        // 5 test cases
        int[] testIndices = {0, 3, 5, 8, 9};

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            int value = getValueFromArray(id_sequence, size, index);
            System.out.println("Value at index " + index + " is " + value);
        }
    }
}