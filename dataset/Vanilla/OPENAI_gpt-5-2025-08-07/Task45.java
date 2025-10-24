import java.util.*;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;

public class Task45 {

    public static String processInput(String input) {
        if (input == null) return "ERROR: Input is null";
        input = input.trim();
        if (input.isEmpty()) return "ERROR: Empty input";

        try {
            String[] parts = input.split("\\s+");
            String op = parts[0].toLowerCase(Locale.ROOT);

            List<Double> nums = new ArrayList<>();
            for (int i = 1; i < parts.length; i++) {
                try {
                    nums.add(Double.parseDouble(parts[i]));
                } catch (NumberFormatException ex) {
                    return "ERROR: Invalid number '" + parts[i] + "'";
                }
            }

            switch (op) {
                case "add": {
                    if (nums.size() < 2) return "ERROR: 'add' requires at least 2 numbers";
                    double sum = 0.0;
                    for (double v : nums) sum += v;
                    if (!Double.isFinite(sum)) return "ERROR: Numeric overflow";
                    return formatDouble(sum);
                }
                case "mul": {
                    if (nums.size() < 2) return "ERROR: 'mul' requires at least 2 numbers";
                    double prod = 1.0;
                    for (double v : nums) prod *= v;
                    if (!Double.isFinite(prod)) return "ERROR: Numeric overflow";
                    return formatDouble(prod);
                }
                case "sub": {
                    if (nums.size() != 2) return "ERROR: 'sub' requires exactly 2 numbers";
                    double res = nums.get(0) - nums.get(1);
                    if (!Double.isFinite(res)) return "ERROR: Numeric overflow";
                    return formatDouble(res);
                }
                case "div": {
                    if (nums.size() != 2) return "ERROR: 'div' requires exactly 2 numbers";
                    double b = nums.get(1);
                    if (b == 0.0) return "ERROR: Division by zero";
                    double res = nums.get(0) / b;
                    if (!Double.isFinite(res)) return "ERROR: Numeric overflow";
                    return formatDouble(res);
                }
                case "pow": {
                    if (nums.size() != 2) return "ERROR: 'pow' requires exactly 2 numbers";
                    double res = Math.pow(nums.get(0), nums.get(1));
                    if (!Double.isFinite(res)) return "ERROR: Numeric overflow";
                    return formatDouble(res);
                }
                default:
                    return "ERROR: Unknown operation '" + op + "'";
            }
        } catch (Exception ex) {
            return "ERROR: Unexpected error";
        }
    }

    private static String formatDouble(double v) {
        double r = Math.rint(v);
        if (Math.abs(v - r) < 1e-12 && Math.abs(r) < 9.22e18) {
            return Long.toString((long) r);
        }
        DecimalFormat df = new DecimalFormat("#.############",
                new DecimalFormatSymbols(Locale.US));
        return df.format(v);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "add 2 3 4",
            "div 10 0",
            "pow 2 10",
            "mul 1.5 4",
            "sub 5 abc"
        };
        for (String t : tests) {
            String out = processInput(t);
            System.out.println(t + " => " + out);
        }
    }
}