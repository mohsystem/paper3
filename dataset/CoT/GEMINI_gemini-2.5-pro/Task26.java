public class Task26 {

    public static int findIt(int[] a) {
        int xor_sum = 0;
        for (int num : a) {
            xor_sum ^= num;
        }
        return xor_sum;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(findIt(new int[]{7})); // Output: 7
        System.out.println(findIt(new int[]{0})); // Output: 0
        System.out.println(findIt(new int[]{1, 1, 2})); // Output: 2
        System.out.println(findIt(new int[]{0, 1, 0, 1, 0})); // Output: 0
        System.out.println(findIt(new int[]{1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1})); // Output: 4
    }
}