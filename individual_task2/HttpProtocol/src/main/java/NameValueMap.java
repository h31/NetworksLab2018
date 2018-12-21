import java.io.*;
import java.util.*;
import java.util.stream.Collectors;


/**
 * Class for working with value=name pairs from the request
 */
public class NameValueMap {
    private List<Header> pairs;

    /**
     * Constructor
     * @param pairs
     */
    private NameValueMap(List<Header> pairs) {
        this.pairs = pairs;
    }


    /**
     * Create new name=value map
     * @return NameValueMap object
     */
    public static NameValueMap createNameValueMap() {
        return new NameValueMap(new ArrayList<>());
    }


    /**
     * Get names from pairs name=value
     * @return name
     */
    public List<String> getNames() {
        return pairs.stream().map(Header::getName).collect(Collectors.toList());
    }


    /**
     * Get values from name=values pairs
     * @param name
     * @return value
     */
    public String getValue(String name) {
        return pairs.stream()
                .filter(header -> header.getName().equals(name))
                .findFirst()
                .orElse(new Header("", ""))
                .getValue();
    }

    /**
     * Add new name=value element
     * @param name
     * @param value
     */
    public void put(String name, String value) {
        pairs.add(new Header(name, value));
    }

    /**
     * Add new name=value element from some string with one divider
     * @param pair
     * @param regex divider element (for example =)
     */
    public void putPair(String pair, String regex) {
        String[] components = pair.split(regex);
        if (components.length == 2) {
            this.pairs.add(new Header(components[0], components[1]));
        }
        if (components.length > 2) {
            StringBuilder value = new StringBuilder();
            for (int i = 1; i < components.length; i++) {
                value.append(components[i]);
            }
            this.pairs.add(new Header(components[0], value.toString()));
        }
    }

    /**
     * Add new name=value element from some string with more than one divider
     * @param pairs - string with pairs
     * @param regexLine divider for pairs
     * @param regexPair divider for name and value
     */
    public void putPairs(String pairs, String regexLine, String regexPair) {
        String[] components, lines;

        lines = pairs.split(regexLine);
        for (int i = 0; i < lines.length; i++) {
            putPair(lines[i], regexPair);
        }
    }

    /**
     * Add more than one pair name=value
     * @param in input stream
     * @param regex divider
     */
    public void putPairs(BufferedReader in, String regex) {
        String line;

        try {
            while (((line = in.readLine()) != null) && !line.equals("")) {
                putPair(line, regex);
            }
        } catch (IOException e) { }
    }

}

/**
 * This class is used for constructing headers
 */
class Header {
    private final String name;
    private final String value;

    /**
     * Constructor
     * @param name
     * @param value
     */
    public Header(String name, String value) {
        this.name = name;
        this.value = value;
    }

    /**
     * Get header name
     * @return name
     */
    public String getName() {
        return name;
    }

    /**
     * Get header value
     * @return value
     */
    public String getValue() {
        return value;
    }
}