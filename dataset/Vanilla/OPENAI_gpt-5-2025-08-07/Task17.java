public class Task17 {
    public static String toJadenCase(String phrase) {
        if (phrase == null || phrase.isEmpty()) return null;
        StringBuilder sb = new StringBuilder(phrase.length());
        boolean newWord = true;
        for (int i = 0; i < phrase.length(); i++) {
            char ch = phrase.charAt(i);
            if (newWord && Character.isLetter(ch)) {
                sb.append(Character.toUpperCase(ch));
            } else if (Character.isLetter(ch)) {
                sb.append(Character.toLowerCase(ch));
            } else {
                sb.append(ch);
            }
            newWord = Character.isWhitespace(ch) || ch == '-';
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "How can mirrors be real if our eyes aren't real",
            "",
            "not jaden-cased string",
            "this IS a tESt",
            "   hello\tworld\nit's me - isn't it?"
        };
        for (String t : tests) {
            String result = toJadenCase(t);
            System.out.println("Input:  " + String.valueOf(t));
            System.out.println("Output: " + String.valueOf(result));
            System.out.println("---");
        }
    }
}