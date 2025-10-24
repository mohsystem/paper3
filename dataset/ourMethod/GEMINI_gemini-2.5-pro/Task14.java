public class Task14 {

    /**
     * Finds an index N in an array of integers where the sum of the integers
     * to the left of N is equal to the sum of the integers to the right of N.
     *
     * @param arr An array of integers.
     * @return The lowest index N where the left side sum equals the right side sum.
     *         Returns -1 if no such index is found.
     */
    public static int findEvenIndex(int[] arr) {
        if (arr == null || arr.length == 0) {
            return -1;
        }

        long totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }

        long leftSum = 0;
        for (int i = 0; i < arr.length; i++) {
            // The right sum is the total sum minus the left sum and the current element
            long rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i;
            }
            
            leftSum += arr[i];
        }

        return -1;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] test1 = {1, 2, 3, 4, 3, 2, 1};
        System.out.println("Input: {1,2,3,4,3,2,1}, Output: " + findEvenIndex(test1));

        // Test Case 2
        int[] test2 = {1, 100, 50, -51, 1, 1};
        System.out.println("Input: {1,100,50,-51,1,1}, Output: " + findEvenIndex(test2));

        // Test Case 3
        int[] test3 = {20, 10, -80, 10, 10, 15, 35};
        System.out.println("Input: {20,10,-80,10,10,15,35}, Output: " + findEvenIndex(test3));

        // Test Case 4: index is at the end
        int[] test4 = {10, -80, 10, 10, 15, 35, 20};
        System.out.println("Input: {10,-80,10,10,15,35,20}, Output: " + findEvenIndex(test4));
        
        // Test Case 5: no solution
        int[] test5 = {1, 2, 3, 4, 5, 6};
        System.out.println("Input: {1,2,3,4,5,6}, Output: " + findEvenIndex(test5));
    }
}