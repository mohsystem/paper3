import java.util.*;

public class Task1 {
    public static int number(int[][] busStops) {
        int passengers = 0;
        for (int[] stop : busStops) {
            passengers += stop[0] - stop[1];
        }
        return passengers;
    }

    public static void main(String[] args) {
        int[][] t1 = {{10, 0}, {3, 5}, {5, 8}};
        int[][] t2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
        int[][] t3 = {{0, 0}};
        int[][] t4 = {{5, 0}, {0, 0}, {0, 0}};
        int[][] t5 = {{21, 0}, {0, 20}, {1, 0}};
        System.out.println(number(t1));
        System.out.println(number(t2));
        System.out.println(number(t3));
        System.out.println(number(t4));
        System.out.println(number(t5));
    }
}