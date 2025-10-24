import java.util.*;

public class Task140 {
    public static double calculate(double a, double b, char op) {
        switch (op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': return b != 0 ? a / b : Double.NaN;
            default: return Double.NaN;
        }
    }

    public static void main(String[] args) {
        System.out.println("3 + 5 = " + calculate(3, 5, '+'));
        System.out.println("10 - 4 = " + calculate(10, 4, '-'));
        System.out.println("7 * 6 = " + calculate(7, 6, '*'));
        System.out.println("20 / 5 = " + calculate(20, 5, '/'));
        System.out.println("7 / 0 = " + calculate(7, 0, '/'));
    }
}