import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.StringJoiner;

public class Task97 {

    /**
     * Executes an XPath query on a given XML file.
     *
     * @param xmlFileName     The path to the XML file.
     * @param xpathExpression The XPath expression to evaluate.
     * @return A string representation of the result, or an error message.
     */
    public static String evaluateXPath(String xmlFileName, String xpathExpression) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.parse(new File(xmlFileName));

            XPathFactory xpathFactory = XPathFactory.newInstance();
            XPath xpath = xpathFactory.newXPath();
            XPathExpression expr = xpath.compile(xpathExpression);

            try {
                // First, try to evaluate as a NODESET, which is the most common case
                NodeList nodes = (NodeList) expr.evaluate(doc, XPathConstants.NODESET);
                if (nodes.getLength() > 0) {
                    StringJoiner joiner = new StringJoiner(", ");
                    for (int i = 0; i < nodes.getLength(); i++) {
                        // getNodeValue() works for text() and attribute() nodes
                        if (nodes.item(i).getNodeValue() != null) {
                           joiner.add(nodes.item(i).getNodeValue());
                        }
                    }
                    return joiner.toString();
                }
            } catch (XPathExpressionException e) {
                // Expression does not evaluate to a NODESET (e.g., count()), fall through
            }

            // Fallback to evaluating as a STRING for scalar results like count()
            // or if the nodeset was empty.
            return (String) expr.evaluate(doc, XPathConstants.STRING);

        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String xmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
                "<bookstore>\n" +
                "    <book id=\"1\">\n" +
                "        <title>The Lord of the Rings</title>\n" +
                "        <author>J.R.R. Tolkien</author>\n" +
                "        <year>1954</year>\n" +
                "    </book>\n" +
                "    <book id=\"2\">\n" +
                "        <title>Pride and Prejudice</title>\n" +
                "        <author>Jane Austen</author>\n" +
                "        <year>1813</year>\n" +
                "    </book>\n" +
                "    <section name=\"fantasy\">\n" +
                "         <book id=\"3\">\n" +
                "            <title>A Game of Thrones</title>\n" +
                "            <author>George R. R. Martin</author>\n" +
                "            <year>1996</year>\n" +
                "        </book>\n" +
                "    </section>\n" +
                "    <book id=\"4\">\n" +
                "        <title>The Hobbit</title>\n" +
                "        <author>J.R.R. Tolkien</author>\n" +
                "        <year>1937</year>\n" +
                "    </book>\n" +
                "</bookstore>";
        String fileName = "test.xml";

        // Create a temporary XML file for testing
        try {
            Files.write(Paths.get(fileName), xmlContent.getBytes());
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
            return;
        }

        // Test cases
        String[][] testCases = {
                {"/bookstore/book[@id='1']/title/text()", "The Lord of the Rings"},
                {"//book[@id='3']/author/text()", "George R. R. Martin"},
                {"/bookstore/book[author='J.R.R. Tolkien']/title/text()", "The Lord of the Rings, The Hobbit"},
                {"count(//book)", "4"},
                {"//section/@name", "fantasy"}
        };

        System.out.println("--- Java Test Cases ---");
        for (String[] testCase : testCases) {
            String xpath = testCase[0];
            String expected = testCase[1];
            String result = evaluateXPath(fileName, xpath);
            System.out.println("XPath: " + xpath);
            System.out.println("Result: " + result);
            System.out.println("Expected: " + expected);
            System.out.println("Status: " + (expected.equals(result) ? "Passed" : "Failed"));
            System.out.println();
        }

        // Clean up the temporary file
        try {
            Files.delete(Paths.get(fileName));
        } catch (IOException e) {
            System.err.println("Failed to delete test file: " + e.getMessage());
        }
    }
}