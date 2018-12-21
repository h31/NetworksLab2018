import java.io.*;
import java.net.*;


/**
 * This class is responsible for receiving and processing the request
 */
public class HttpRequest extends HttpMessage {
    private NameValueMap queryParameters, queryPost;
    private String method, queryString, version;
    private URI uri;

    /**
     * This is the default constructor of the class
     */
    public HttpRequest() {
        super();
        method = null;
        queryString = null;
        version = null;
    }

    /**
     * This function returns the name of http method
     * @return method
     */
    public String getMethod() {
        return method;
    }

    /**
     * This method receives and processes URI from client's request
     * if the client didn't specify a path in his request, method returns "/index.html"
     * by default
     * @return String path - URI path
     */
    public String getRequestURI() {
        String path;

        if (uri == null) {
            path = null;
        }
        else {
            path = uri.getPath();
        }

        if (path == null || path.equals("") || path.equals("/")) {
            return "/index.html";
        }
        else {
            return path;
        }
    }

    /**
     * This method reads and processes client's request
     * @param reader - input stream
     * @throws IOException
     * @throws URISyntaxException
     */
    public void read(BufferedReader reader) throws IOException, URISyntaxException {

        String httpLine = reader.readLine();

        String[] tokens = httpLine.split("\\s");
        method = tokens[0];
        String request = tokens[1];

        if (tokens.length > 2) {
            version = tokens[2];
        }
        else {
            version = "HTTP/1.1";
        }

        // Parse the URI
        uri = new URI(request);

        // Get the decoded query string
        queryString = uri.getQuery();

        // Process the query string
        queryParameters = NameValueMap.createNameValueMap();
        if (queryString != null) {
            queryParameters.putPairs(queryString, "&", "=");
        }

        // Process the headers
        headers = NameValueMap.createNameValueMap();
        headers.putPairs(reader, ":");


        if(getContentLength() != -1 && method.equals("POST")) {
            char[] content = new char[getContentLength()];
            if(reader.read(content) != -1) {
                queryPost = NameValueMap.createNameValueMap();
                queryPost.putPairs(new String(content), "&", "=");
            } else {
                queryPost = null;
            }
        } else {
            readContent(reader);
        }
    }

    /**
     * This method is used for getting query of POST type - it may be some user's data
     * @return queryPost
     */
    NameValueMap getQueryPost() {
        return queryPost;
    }

    /**
     * Converts to string request fields
     * @return string
     */
    public String toString() {
        String s = "Method: \n\t" + getMethod() + "\n";
        s += "URI: \n\t" + getRequestURI() + "\n";

        s += "Parameters:\n" + queryString + "\n";

        s += "Headers:\n";
        for (String name : getHeaderNames()) {
            String value = headers.getValue(name);
            s += "\t" + name + "\t" + value + "\n";
        }

        return s;
    }
}