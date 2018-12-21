import java.io.*;

/**
 * This class is responsible for the formation of the response
 */
public class HttpResponse extends HttpMessage {
    private int status;

    //Here are basic http statuses
    public static final int SC_ACCEPTED = 202;
    public static final int SC_BAD_GATEWAY = 502;
    public static final int SC_BAD_REQUEST = 400;
    public static final int SC_CREATED = 201;
    public static final int SC_FORBIDDEN = 403;
    public static final int SC_INTERNAL_ERROR = 500;
    public static final int SC_MOVED = 301;
    public static final int SC_NO_RESPONSE = 204;
    public static final int SC_NOT_FOUND = 404;
    public static final int SC_NOT_IMPLEMENTED = 501;
    public static final int SC_OK = 200;
    public static final int SC_PARTIAL_INFORMATION = 203;
    public static final int SC_PAYMENT_REQUIRED = 402;
    public static final int SC_SERVICE_OVERLOADED = 503;
    public static final int SC_UNAUTHORIZED = 401;

    private static final String VERSION = "HTTP/1.1 ";

    /**
     * This is the default constructor of the class
     */
    public HttpResponse() {
        super();
        status = SC_OK;
    }

    /**
     * This method returns the status code
     * @return status
     */
    public int getStatus() {
        return status;
    }


    /**
     * This method associates status code with status comment
     * @param statusCode - status code
     * @return string - status code comment
     */
    public static String getStatusMessage(int statusCode) {
        switch (statusCode) {
            case SC_ACCEPTED:
                return "Accepted";
            case SC_BAD_GATEWAY:
                return "Bad Gateway";
            case SC_BAD_REQUEST:
                return "Bad Request";
            case SC_CREATED:
                return "Created";
            case SC_FORBIDDEN:
                return "Forbidden";
            case SC_INTERNAL_ERROR:
                return "Internal Error";
            case SC_MOVED:
                return "Moved";
            case SC_NO_RESPONSE:
                return "No Response";
            case SC_NOT_FOUND:
                return "Not Found";
            case SC_NOT_IMPLEMENTED:
                return "Not Implemented";
            case SC_OK:
                return "OK";
            case SC_PARTIAL_INFORMATION:
                return "Partial Information";
            case SC_PAYMENT_REQUIRED:
                return "Payment Required";
            case SC_SERVICE_OVERLOADED:
                return "Service Overloaded";
            case SC_UNAUTHORIZED:
                return "Unauthorized";
            default:
                return "Unknown Status Code " + statusCode;
        }
    }


    public void read(BufferedReader in) {
        //TODO ("not implement")
    }

    /**
     * This method sends an error message if client's request is not correct
     * @param statusCode - bad status code
     * @return string - error message
     */
    public String sendError(int statusCode) {
        String errorHTML = "<HTML><BODY><P>HTTP Error " + statusCode + " - " +
                getStatusMessage(statusCode) +
                "</P></BODY></HTML>\r\n";

        return send(statusCode, errorHTML);
    }

    /**
     * This method sends an positive message if client's request is correct
     * @return string with positive message
     */
    public String sendOK() {
        String errorHTML = "<HTML><BODY><P>OK " + SC_OK + " - " +
                getStatusMessage(SC_OK) +
                "</P></BODY></HTML>\r\n";

        return send(SC_OK, errorHTML);
    }

    /**
     * This method forms a message about status of the request
     * @param statusCode - status code
     * @param content - negative or positive message
     * @return
     */
    public String send(int statusCode, String content) {
        setStatus(statusCode);
        setContent(content.getBytes());

        return write();
    }

    public String send(int statusCode, byte[] content) {
        setStatus(statusCode);
        setContent(content);

        return write();
    }

    public String sendResponseHeader(int statusCode, long contentLength) {
        setStatus(statusCode);
        setContentLength(contentLength);

        return write();
    }

    /**
     * Method sets a status code for the response
     * @param statusCode
     */
    public void setStatus(int statusCode) {
        status = statusCode;
    }

    /**
     * Converts to string response fields
     * @return data in string format
     */
    public String toString() {
        String s = VERSION + " " + status + " " + getStatusMessage(status) + "\r\n";

        for (String name : getHeaderNames()) {
            String value = headers.getValue(name);
            s += name + ": " + value + "\r\n";
        }
        s += "\r\n";

        s += new String(getContent());

        return s;
    }

    /**
     * This method is used for writing http response
     * @return string - response
     */
    public String write() {
        if (content != null) {
            setContentLength(content.length);
        }
        else {
            setContentLength(0);
        }

        String result = writeStatusLine();
        result += writeHeaders();

        if (content != null) {
            result += new String(content);
        }
        return result;
    }

    /**
     * This method writes the headers
     * @return headers in string format
     */
    private String writeHeaders() {
        StringBuilder sb = new StringBuilder();
        for (String name : headers.getNames()) {
            String value = headers.getValue(name);

            if (value != null && !value.equals("")) {
                sb.append(name).append(": ").append(value).append("\r\n");
            }
        }
        sb.append("\r\n");
        return sb.toString();
    }

    /**
     * Generates a status line
     * @return string - status line
     */
    private String writeStatusLine() {
        return VERSION + status + " " + getStatusMessage(status) + "\r\n";
    }
}