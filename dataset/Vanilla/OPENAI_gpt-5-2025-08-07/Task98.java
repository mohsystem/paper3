import java.util.Scanner;

public class Task98 {
    public static Integer tryParseInt(String s) {
        if (s == null) return null;
        s = s.trim();
        try {
            return Integer.valueOf(s);
        } catch (NumberFormatException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        for (int i = 1; i <= 5; i++) {
            System.out.print("Enter value #" + i + ": ");
            if (!sc.hasNextLine()) break;
            String input = sc.nextLine();
            Integer result = tryParseInt(input);
            if (result != null) {
                System.out.println("Parsed integer: " + result);
            } else {
                System.out.println("Invalid integer");
            }
        }
        sc.close();
    }
}