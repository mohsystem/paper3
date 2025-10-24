
import java.util.*;

public class Task169 {
    private static class TreeNode {
        int val;
        int count;
        int leftSize;
        TreeNode left;
        TreeNode right;
        
        TreeNode(int val) {
            this.val = val;
            this.count = 1;
            this.leftSize = 0;
        }
    }
    
    public static List<Integer> countSmaller(int[] nums) {
        if (nums == null || nums.length == 0) {
            return new ArrayList<>();
        }
        
        Integer[] result = new Integer[nums.length];
        TreeNode root = null;
        
        for (int i = nums.length - 1; i >= 0; i--) {
            root = insert(root, nums[i], result, i, 0);
        }
        
        return Arrays.asList(result);
    }
    
    private static TreeNode insert(TreeNode node, int val, Integer[] result, int index, int preSum) {
        if (node == null) {
            node = new TreeNode(val);
            result[index] = preSum;
        } else if (node.val == val) {
            node.count++;
            result[index] = preSum + node.leftSize;
        } else if (node.val > val) {
            node.leftSize++;
            node.left = insert(node.left, val, result, index, preSum);
        } else {
            node.right = insert(node.right, val, result, index, preSum + node.leftSize + node.count);
        }
        return node;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1 = {5, 2, 6, 1};
        System.out.println("Test 1: " + countSmaller(nums1));
        
        // Test case 2
        int[] nums2 = {-1};
        System.out.println("Test 2: " + countSmaller(nums2));
        
        // Test case 3
        int[] nums3 = {-1, -1};
        System.out.println("Test 3: " + countSmaller(nums3));
        
        // Test case 4
        int[] nums4 = {1, 2, 3, 4, 5};
        System.out.println("Test 4: " + countSmaller(nums4));
        
        // Test case 5
        int[] nums5 = {5, 4, 3, 2, 1};
        System.out.println("Test 5: " + countSmaller(nums5));
    }
}
