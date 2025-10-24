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

    private int[] counts;

    public List<Integer> countSmaller(int[] nums) {
        if (nums == null || nums.length == 0) {
            return new ArrayList<>();
        }

        int n = nums.length;
        this.counts = new int[n];
        Pair[] items = new Pair[n];
        for (int i = 0; i < n; i++) {
            items[i] = new Pair(nums[i], i);
        }

        mergeSort(items, 0, n - 1);

        List<Integer> result = new ArrayList<>(n);
        for (int count : counts) {
            result.add(count);
        }
        return result;
    }

    private void mergeSort(Pair[] items, int start, int end) {
        if (start >= end) {
            return;
        }
        int mid = start + (end - start) / 2;
        mergeSort(items, start, mid);
        mergeSort(items, mid + 1, end);
        merge(items, start, mid, end);
    }

    private void merge(Pair[] items, int start, int mid, int end) {
        Pair[] temp = new Pair[end - start + 1];
        int i = start;
        int j = mid + 1;
        int k = 0;

        while (i <= mid && j <= end) {
            if (items[i].val <= items[j].val) {
                counts[items[i].index] += j - (mid + 1);
                temp[k++] = items[i++];
            } else {
                temp[k++] = items[j++];
            }
        }

        while (i <= mid) {
            counts[items[i].index] += j - (mid + 1);
            temp[k++] = items[i++];
        }

        while (j <= end) {
            temp[k++] = items[j++];
        }

        System.arraycopy(temp, 0, items, start, temp.length);
    }

    public static void main(String[] args) {
        Task169 solution = new Task169();
        
        // Test Case 1
        int[] nums1 = {5, 2, 6, 1};
        System.out.println("Test Case 1:");
        System.out.println("Input: " + Arrays.toString(nums1));
        System.out.println("Output: " + solution.countSmaller(nums1)); // Expected: [2, 1, 1, 0]

        // Test Case 2
        int[] nums2 = {-1};
        System.out.println("\nTest Case 2:");
        System.out.println("Input: " + Arrays.toString(nums2));
        System.out.println("Output: " + solution.countSmaller(nums2)); // Expected: [0]

        // Test Case 3
        int[] nums3 = {-1, -1};
        System.out.println("\nTest Case 3:");
        System.out.println("Input: " + Arrays.toString(nums3));
        System.out.println("Output: " + solution.countSmaller(nums3)); // Expected: [0, 0]
        
        // Test Case 4
        int[] nums4 = {};
        System.out.println("\nTest Case 4:");
        System.out.println("Input: " + Arrays.toString(nums4));
        System.out.println("Output: " + solution.countSmaller(nums4)); // Expected: []

        // Test Case 5
        int[] nums5 = {2, 0, 1};
        System.out.println("\nTest Case 5:");
        System.out.println("Input: " + Arrays.toString(nums5));
        System.out.println("Output: " + solution.countSmaller(nums5)); // Expected: [2, 0, 0]
    }
}