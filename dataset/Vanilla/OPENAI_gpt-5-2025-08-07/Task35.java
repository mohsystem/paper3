import java.util.*;

public class Task35 {
    // Function to compute remaining people on the bus
    public static int countPeople(int[][] stops) {
        int total = 0;
        for (int[] stop : stops) {
            total += stop[0] - stop[1];
        }
        return total;
    }

    public static void main(String[] args) {
        int[][] test1 = { {10, 0}, {3, 5}, {5, 8} }; // Expected: 5
        int[][] test2 = { {3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10} }; // Expected: 17
        int[][] test3 = { {0, 0} }; // Expected: 0
        int[][] test4 = { {5, 0}, {5, 5}, {10, 10}, {1, 1} }; // Expected: 0
        int[][] test5 = { {50, 0}, {30, 10}, {5, 5}, {40, 20}, {0, 0}, {1, 1} }; // Expected: 90

        System.out.println(countPeople(test1));
        System.out.println(countPeople(test2));
        System.out.println(countPeople(test3));
        System.out.println(countPeople(test4));
        System.out.println(countPeople(test5));
    }
}