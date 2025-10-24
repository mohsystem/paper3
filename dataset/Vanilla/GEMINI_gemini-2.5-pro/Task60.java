public class Task60 {

    public static String expandedForm(int num) {
        String s = String.valueOf(num);
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char digit = s.charAt(i);
            if (digit != '0') {
                if (result.length() > 0) {
                    result.append(" + ");
                }
                result.append(digit);
                for (int j = 0; j < s.length() - 1 - i; j++) {
                    result.append('0');
                }
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        System.out.println("12 -> " + expandedForm(12));
        System.out.println("42 -> " + expandedForm(42));
        System.out.println("70304 -> " + expandedForm(70304));
        System.out.println("999 -> " + expandedForm(999));
        System.out.println("800000 -> " + expandedForm(800000));
    }
}