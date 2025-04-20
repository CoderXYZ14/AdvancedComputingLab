import java.io.*;
import java.net.*;
import java.util.Arrays;

public class Server {
    private static final int PORT = 5000;

    public static void main(String[] args) {
        System.out.println("Starting Statistics Server on port " + PORT);

        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("Server is listening on port " + PORT);

            while (true) {
                try (
                        Socket socket = serverSocket.accept();
                        ObjectInputStream inputStream = new ObjectInputStream(socket.getInputStream());
                        ObjectOutputStream outputStream = new ObjectOutputStream(socket.getOutputStream());) {
                    System.out.println("Client connected: " + socket.getInetAddress().getHostAddress());

                    // Receive data from client
                    double[] data = (double[]) inputStream.readObject();
                    System.out.println("Received data: " + Arrays.toString(data));

                    // Check if data is valid
                    if (data == null || data.length == 0) {
                        outputStream.writeObject(new StatisticsResult(0, 0, "Error: Empty data set"));
                        continue;
                    }

                    // Calculate statistics
                    StatisticsCalculator calculator = new StatisticsCalculator(data);
                    double mean = calculator.calculateMean();
                    double stdDev = calculator.calculateStandardDeviation();

                    // Send result back to client
                    StatisticsResult result = new StatisticsResult(mean, stdDev, "Success");
                    outputStream.writeObject(result);

                    System.out.println("Sent results to client - Mean: " + mean + ", StdDev: " + stdDev);

                } catch (ClassNotFoundException e) {
                    System.err.println("Error in data format: " + e.getMessage());
                } catch (EOFException e) {
                    System.err.println("Client disconnected");
                } catch (Exception e) {
                    System.err.println("Server error: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        } catch (IOException e) {
            System.err.println("Server exception: " + e.getMessage());
            e.printStackTrace();
        }
    }
}

// Class for statistical calculations
class StatisticsCalculator {
    private double[] data;

    public StatisticsCalculator(double[] data) {
        this.data = data;
    }

    public double calculateMean() {
        double sum = 0;
        for (double value : data) {
            sum += value;
        }
        return sum / data.length;
    }

    public double calculateStandardDeviation() {
        double mean = calculateMean();
        double sumOfSquaredDifferences = 0;

        for (double value : data) {
            double difference = value - mean;
            sumOfSquaredDifferences += difference * difference;
        }

        double variance = sumOfSquaredDifferences / data.length;
        return Math.sqrt(variance);
    }
}

// Class for sending results back to client
class StatisticsResult implements Serializable {
    private static final long serialVersionUID = 1L;

    private double mean;
    private double standardDeviation;
    private String status;

    public StatisticsResult(double mean, double standardDeviation, String status) {
        this.mean = mean;
        this.standardDeviation = standardDeviation;
        this.status = status;
    }

    public double getMean() {
        return mean;
    }

    public double getStandardDeviation() {
        return standardDeviation;
    }

    public String getStatus() {
        return status;
    }
}