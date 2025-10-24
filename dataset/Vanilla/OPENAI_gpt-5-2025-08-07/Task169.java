import java.util.*;
public class Task169 {
    public static int[] countSmaller(int[] nums) {
        int n = nums.length;
        int[] res = new int[n];
        if (n == 0) return res;

        int[] sorted = nums.clone();
        Arrays.sort(sorted);
        int m = 0;
        int[] uniq = new int[n];
        for (int x : sorted) {
            if (m == 0 || uniq[m - 1] != x) uniq[m++] = x;
        }
        HashMap<Integer, Integer> idx = new HashMap<>();
        for (int i = 0; i < m; i++) idx.put(uniq[i], i + 1);

        int[] bit = new int[m + 2];

        for (int i = n - 1; i >= 0; i--) {
            int id = idx.get(nums[i]);
            res[i] = query(bit, id - 1);
            update(bit, id, 1, m + 1);
        }

        return res;
    }

    private static void update(int[] bit, int i, int delta, int size) {
        for (; i <= size; i += i & -i) bit[i] += delta;
    }

    private static int query(int[] bit, int i) {
        int s = 0;
        for (; i > 0; i -= i & -i) s += bit[i];
        return s;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        int[][] tests = {
            {5,2,6,1},
            {-1},
            {-1,-1},
            {3,2,2,6,1},
            {1,0,2,2,5}
        };
        for (int[] t : tests) {
            printArray(countSmaller(t));
        }
    }
}