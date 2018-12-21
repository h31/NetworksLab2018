import java.io.*;
import java.net.*;
import java.util.concurrent.*;
import java.util.logging.*;

/**
 * This is start server class
 */
public class HttpServer implements Runnable {

    private volatile boolean keepRunning;
    private final ServerSocket serverSocket;

    private final ExecutorService threadPool;
    private static final int MAX_THREADS = 100;

    private static Logger logger;

    public static void main(String[] args) {

        logger = Logger.getLogger(HttpServer.class.getName());
        System.setProperty("java.util.logging.SimpleFormatter.format", "%1$tF %1$tT %4$s %2$s %5$s%6$s%n");

        try {
            BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

            // Start the server
            HttpServer server = new HttpServer();
            server.start();

            System.out.println("Press [Enter] to stop the server...");

            // Block until the user presses [Enter]
            in.readLine();
            server.stop();
        } catch (IOException e) {
            logger.log(Level.SEVERE, e.getMessage(), e);
        }
    }

    /**
     * This is default constructor
     * @throws IOException
     */
    private HttpServer() throws IOException {
        serverSocket = new ServerSocket(8080);
        logger.info("Created Server Socket on 8080");

        threadPool = Executors.newFixedThreadPool(MAX_THREADS);
        serverSocket.setSoTimeout(0);
    }

    /**
     * Run server's thread and start connection
     */
    @Override
    public void run() {
        while (keepRunning) {
            try {
                Socket s = serverSocket.accept();
                logger.info("Accepted a connection: " + s.toString());
                HttpConnectionHandler connection = new HttpConnectionHandler(s);

                threadPool.execute(connection);
            } catch (SocketTimeoutException e) {

                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        stopPool();
    }

    /**
     * Stop all threads in the pool
     */
    private void stopPool() {

        threadPool.shutdown();

        try {
            // Wait for existing connections to complete
            if (!threadPool.awaitTermination(5, TimeUnit.SECONDS)) {
                // Stop executing threads
                threadPool.shutdownNow();

                if (!threadPool.awaitTermination(5, TimeUnit.SECONDS)) {
                    logger.log(Level.INFO, "Could not stop thread pool.");
                }
            }
        } catch (InterruptedException ie) {

            threadPool.shutdownNow();
        }
    }

    /**
     * Start server's thread
     */
    private void start() {
        keepRunning = true;
        new Thread(this).start();
    }


    /**
     * Stop server's thread
     * @throws IOException
     */
    private void stop() throws IOException {
        keepRunning = false;
        serverSocket.close();
    }
}