import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task169 {

    private static class Pair {
        int val;
        int index;

        Pair(int val, int index) {
            this.val = val;
            this.index = index;
        }
    }

    public List<Integer> countSmaller(int[] nums) {
        if (nums == null || nums.length == 0) {
            return new ArrayList<>();
        }

        int n = nums.length;
        Pair[] pairs = new Pair[n];
        for (int i = 0; i < n; i++) {
            pairs[i] = new Pair(nums[i], i);
        }

        int[] counts = new int[n];
        mergeSort(pairs, 0, n - 1, counts);

        List<Integer> result = new ArrayList<>(n);
        for (int count : counts) {
            result.add(count);
        }
        return result;
    }

    private void mergeSort(Pair[] pairs, int start, int end, int[] counts) {
        if (start >= end) {
            return;
        }

        int mid = start + (end - start) / 2;
        mergeSort(pairs, start, mid, counts);
        mergeSort(pairs, mid + 1, end, counts);
        merge(pairs, start, mid, end, counts);
    }

    private void merge(Pair[] pairs, int start, int mid, int end, int[] counts) {
        int tempSize = end - start + 1;
        Pair[] temp = new Pair[tempSize];
        
        int i = start;        // pointer for the left half
        int j = mid + 1;      // pointer for the right half
        int k = 0;            // pointer for the temporary array
        int rightElementsSmaller = 0;

        while (i <= mid && j <= end) {
            if (pairs[i].val > pairs[j].val) {
                temp[k++] = pairs[j++];
                rightElementsSmaller++;
            } else {
                counts[pairs[i].index] += rightElementsSmaller;
                temp[k++] = pairs[i++];
            }
        }

        while (i <= mid) {
            counts[pairs[i].index] += rightElementsSmaller;
            temp[k++] = pairs[i++];
        }

        while (j <= end) {
            temp[k++] = pairs[j++];
        }

        System.arraycopy(temp, 0, pairs, start, tempSize);
    }

    public static void main(String[] args) {
        Task169 solution = new Task169();

        // Test Case 1
        int[] nums1 = {5, 2, 6, 1};
        System.out.println("Test Case 1: " + solution.countSmaller(nums1));

        // Test Case 2
        int[] nums2 = {-1};
        System.out.println("Test Case 2: " + solution.countSmaller(nums2));

        // Test Case 3
        int[] nums3 = {-1, -1};
        System.out.println("Test Case 3: " + solution.countSmaller(nums3));

        // Test Case 4
        int[] nums4 = {};
        System.out.println("Test Case 4: " + solution.countSmaller(nums4));

        // Test Case 5
        int[] nums5 = {2, 0, 1};
        System.out.println("Test Case 5: " + solution.countSmaller(nums5));
    }
}