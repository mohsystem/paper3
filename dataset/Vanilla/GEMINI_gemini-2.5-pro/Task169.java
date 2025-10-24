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
        Pair[] items = new Pair[n];
        for (int i = 0; i < n; i++) {
            items[i] = new Pair(nums[i], i);
        }

        int[] counts = new int[n];
        mergeSort(items, 0, n - 1, counts);

        List<Integer> result = new ArrayList<>(n);
        for (int count : counts) {
            result.add(count);
        }
        return result;
    }

    private void mergeSort(Pair[] items, int start, int end, int[] counts) {
        if (start >= end) {
            return;
        }
        int mid = start + (end - start) / 2;
        mergeSort(items, start, mid, counts);
        mergeSort(items, mid + 1, end, counts);
        merge(items, start, mid, end, counts);
    }

    private void merge(Pair[] items, int start, int mid, int end, int[] counts) {
        Pair[] temp = new Pair[end - start + 1];
        int i = start;
        int j = mid + 1;
        int k = 0;
        int rightCounter = 0;

        while (i <= mid && j <= end) {
            if (items[i].val <= items[j].val) {
                counts[items[i].index] += rightCounter;
                temp[k++] = items[i++];
            } else {
                rightCounter++;
                temp[k++] = items[j++];
            }
        }

        while (i <= mid) {
            counts[items[i].index] += rightCounter;
            temp[k++] = items[i++];
        }

        while (j <= end) {
            temp[k++] = items[j++];
        }

        for (int l = 0; l < temp.length; l++) {
            items[start + l] = temp[l];
        }
    }

    public static void main(String[] args) {
        Task169 solution = new Task169();

        // Test Case 1
        int[] nums1 = {5, 2, 6, 1};
        System.out.println("Test Case 1:");
        System.out.println("Input: " + Arrays.toString(nums1));
        System.out.println("Output: " + solution.countSmaller(nums1));

        // Test Case 2
        int[] nums2 = {-1};
        System.out.println("\nTest Case 2:");
        System.out.println("Input: " + Arrays.toString(nums2));
        System.out.println("Output: " + solution.countSmaller(nums2));

        // Test Case 3
        int[] nums3 = {-1, -1};
        System.out.println("\nTest Case 3:");
        System.out.println("Input: " + Arrays.toString(nums3));
        System.out.println("Output: " + solution.countSmaller(nums3));

        // Test Case 4
        int[] nums4 = {2, 0, 1};
        System.out.println("\nTest Case 4:");
        System.out.println("Input: " + Arrays.toString(nums4));
        System.out.println("Output: " + solution.countSmaller(nums4));
        
        // Test Case 5
        int[] nums5 = {};
        System.out.println("\nTest Case 5:");
        System.out.println("Input: " + Arrays.toString(nums5));
        System.out.println("Output: " + solution.countSmaller(nums5));
    }
}