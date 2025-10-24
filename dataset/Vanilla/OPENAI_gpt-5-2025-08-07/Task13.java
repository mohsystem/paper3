import java.util.*;

public class Task13 {
    public static boolean isValidWalk(char[] walk) {
        if (walk == null || walk.length != 10) return false;
        int x = 0, y = 0;
        for (char c : walk) {
            switch (c) {
                case 'n': y++; break;
                case 's': y--; break;
                case 'e': x++; break;
                case 'w': x--; break;
                default: return false;
            }
        }
        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        char[] w1 = new char[]{'n','s','n','s','e','w','e','w','n','s'}; // true
        char[] w2 = new char[]{'n','s','e'}; // false (length != 10)
        char[] w3 = new char[]{'n','n','n','n','n','s','s','s','e','w'}; // false (not back to start)
        char[] w4 = new char[]{'e','w','e','w','n','s','n','s','n','s'}; // true
        char[] w5 = new char[]{'n','n','n','n','n','n','n','n','n','n'}; // false

        System.out.println(isValidWalk(w1));
        System.out.println(isValidWalk(w2));
        System.out.println(isValidWalk(w3));
        System.out.println(isValidWalk(w4));
        System.out.println(isValidWalk(w5));
    }
}