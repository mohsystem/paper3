import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task169 {

    private static class Item {
        int val;
        int index;

        Item(int val, int index) {
            this.val = val;
            this.index = index;
        }
    }

    private void mergeSort(Item[] items, int[] counts, int low, int high) {
        if (low >= high) {
            return;
        }
        int mid = low + (high - low) / 2;
        mergeSort(items, counts, low, mid);
        mergeSort(items, counts, mid + 1, high);
        merge(items, counts, low, mid, high);
    }

    private void merge(Item[] items, int[] counts, int low, int mid, int high) {
        int leftSize = mid - low + 1;
        int rightSize = high - mid;
        
        Item[] left = new Item[leftSize];
        Item[] right = new Item[rightSize];
        
        System.arraycopy(items, low, left, 0, leftSize);
        System.arraycopy(items, mid + 1, right, 0, rightSize);

        int i = 0; // Pointer for left array
        int j = 0; // Pointer for right array
        int k = low; // Pointer for original items array

        while (i < leftSize && j < rightSize) {
            if (left[i].val <= right[j].val) {
                counts[left[i].index] += j;
                items[k++] = left[i++];
            } else {
                items[k++] = right[j++];
            }
        }

        while (i < leftSize) {
            counts[left[i].index] += rightSize;
            items[k++] = left[i++];
        }

        while (j < rightSize) {
            items[k++] = right[j++];
        }
    }

    public List<Integer> countSmaller(int[] nums) {
        if (nums == null || nums.length == 0) {
            return new ArrayList<>();
        }
        int n = nums.length;
        Item[] items = new Item[n];
        for (int i = 0; i < n; i++) {
            items[i] = new Item(nums[i], i);
        }

        int[] counts = new int[n];
        mergeSort(items, counts, 0, n - 1);

        List<Integer> result = new ArrayList<>(n);
        for (int count : counts) {
            result.add(count);
        }
        return result;
    }
    
    public static void main(String[] args) {
        Task169 solution = new Task169();

        // Test Case 1
        int[] nums1 = {5, 2, 6, 1};
        System.out.println("Test Case 1: nums = " + Arrays.toString(nums1) + ", Output: " + solution.countSmaller(nums1));

        // Test Case 2
        int[] nums2 = {-1};
        System.out.println("Test Case 2: nums = " + Arrays.toString(nums2) + ", Output: " + solution.countSmaller(nums2));

        // Test Case 3
        int[] nums3 = {-1, -1};
        System.out.println("Test Case 3: nums = " + Arrays.toString(nums3) + ", Output: " + solution.countSmaller(nums3));

        // Test Case 4
        int[] nums4 = {5, 4, 3, 2, 1};
        System.out.println("Test Case 4: nums = " + Arrays.toString(nums4) + ", Output: " + solution.countSmaller(nums4));

        // Test Case 5
        int[] nums5 = {};
        System.out.println("Test Case 5: nums = " + Arrays.toString(nums5) + ", Output: " + solution.countSmaller(nums5));
    }
}