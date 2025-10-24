import java.util.Arrays;

public class Task143 {
    public static int[] sortArray(int[] arr) {
        int[] res = Arrays.copyOf(arr, arr.length);
        Arrays.sort(res);
        return res;
    }

    public static void main(String[] args) {
        int[] a1 = {3, 1, 2};
        int[] a2 = {5, -1, 4, 4, 2};
        int[] a3 = {};
        int[] a4 = {10};
        int[] a5 = {9, 8, 7, 6, 5, 4, 3, 2, 1};

        System.out.println(Arrays.toString(sortArray(a1)));
        System.out.println(Arrays.toString(sortArray(a2)));
        System.out.println(Arrays.toString(sortArray(a3)));
        System.out.println(Arrays.toString(sortArray(a4)));
        System.out.println(Arrays.toString(sortArray(a5)));
    }
}