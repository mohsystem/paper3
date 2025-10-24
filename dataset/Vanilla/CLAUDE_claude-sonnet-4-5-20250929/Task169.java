
import java.util.*;

public class Task169 {
    private static class Node {
        int val;
        int count;
        Node left, right;
        
        Node(int val) {
            this.val = val;
            this.count = 0;
        }
    }
    
    public static List<Integer> countSmaller(int[] nums) {
        Integer[] result = new Integer[nums.length];
        if (nums == null || nums.length == 0) {
            return Arrays.asList(result);
        }
        
        Node root = null;
        for (int i = nums.length - 1; i >= 0; i--) {
            root = insert(root, nums[i], result, i, 0);
        }
        
        return Arrays.asList(result);
    }
    
    private static Node insert(Node node, int val, Integer[] result, int index, int preSum) {
        if (node == null) {
            node = new Node(val);
            result[index] = preSum;
        } else if (node.val > val) {
            node.count++;
            node.left = insert(node.left, val, result, index, preSum);
        } else {
            node.right = insert(node.right, val, result, index, preSum + node.count + (node.val < val ? 1 : 0));
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
