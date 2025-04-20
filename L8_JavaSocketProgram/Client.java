import java.awt.*;
import java.io.*;
import java.net.*;
import java.text.DecimalFormat;
import java.util.ArrayList;
import javax.swing.*;
import javax.swing.border.*;

public class Client extends JFrame {
    private static final long serialVersionUID = 1L;
    private static final int PORT = 5000;
    private static final String SERVER_ADDRESS = "localhost"; // Change to server IP when running on different machines

    private JTextField inputField;
    private JTextArea dataListArea;
    private JButton addButton;
    private JButton calculateButton;
    private JButton clearButton;
    private JLabel statusLabel;
    private JLabel meanLabel;
    private JLabel stdDevLabel;

    private ArrayList<Double> dataList;
    private DecimalFormat df;

    public Client() {
        dataList = new ArrayList<>();
        df = new DecimalFormat("#.####");

        // Set up the frame
        setTitle("Statistics Calculator");
        setSize(600, 500); // Increased window size
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);

        // Create components with modern look
        setupComponents();

        // Create layouts
        setupLayout();

        // Add action listeners
        setupListeners();

        // Display the window
        setVisible(true);
    }

    private void setupComponents() {
        // Input components - increased size
        inputField = new JTextField(15); // Increased width
        inputField.setFont(new Font("Segoe UI", Font.PLAIN, 16)); // Larger font

        // Buttons with modern look - black text
        addButton = createStyledButton("Add Value");
        calculateButton = createStyledButton("Calculate Statistics");
        clearButton = createStyledButton("Clear All");

        // Results display area - increased size
        dataListArea = new JTextArea(12, 25); // Increased rows and columns
        dataListArea.setEditable(false);
        dataListArea.setFont(new Font("Segoe UI", Font.PLAIN, 16)); // Larger font
        dataListArea.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createTitledBorder(
                        BorderFactory.createLineBorder(new Color(200, 200, 200), 2, true), // Thicker border
                        "Data Values",
                        TitledBorder.LEFT,
                        TitledBorder.TOP,
                        new Font("Segoe UI", Font.BOLD, 14)), // Larger title font
                BorderFactory.createEmptyBorder(8, 8, 8, 8))); // Wider padding

        // Status and results labels
        statusLabel = createResultLabel("Status: Ready");
        meanLabel = createResultLabel("Mean: —");
        stdDevLabel = createResultLabel("Standard Deviation: —");
    }

    private JButton createStyledButton(String text) {
        JButton button = new JButton(text);
        button.setFont(new Font("Segoe UI", Font.BOLD, 14)); // Larger font
        button.setFocusPainted(false);
        button.setBorderPainted(true);
        button.setBackground(new Color(70, 130, 180));
        button.setForeground(Color.BLACK); // Changed to black text
        button.setPreferredSize(new Dimension(180, 35)); // Larger button size
        return button;
    }

    private JLabel createResultLabel(String text) {
        JLabel label = new JLabel(text);
        label.setFont(new Font("Segoe UI", Font.PLAIN, 16)); // Larger font
        label.setBorder(BorderFactory.createEmptyBorder(8, 0, 8, 0)); // More vertical space
        return label;
    }

    private void setupLayout() {
        setLayout(new BorderLayout(15, 15)); // Increased gap
        JPanel contentPanel = new JPanel(new BorderLayout(15, 15)); // Increased gap
        contentPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20)); // Wider padding

        // Input panel (top)
        JPanel inputPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 10)); // Added spacing
        JLabel inputLabel = new JLabel("Enter value:");
        inputLabel.setFont(new Font("Segoe UI", Font.BOLD, 16)); // Larger font
        inputPanel.add(inputLabel);
        inputPanel.add(inputField);
        inputPanel.add(addButton);

        // Center panel with data list
        JScrollPane scrollPane = new JScrollPane(dataListArea);
        scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

        // Button panel (below data list)
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 15, 10)); // Added spacing
        buttonPanel.add(calculateButton);
        buttonPanel.add(clearButton);

        // Results panel (bottom)
        JPanel resultsPanel = new JPanel();
        resultsPanel.setLayout(new BoxLayout(resultsPanel, BoxLayout.Y_AXIS));
        resultsPanel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createTitledBorder(
                        BorderFactory.createLineBorder(new Color(200, 200, 200), 2, true), // Thicker border
                        "Results",
                        TitledBorder.LEFT,
                        TitledBorder.TOP,
                        new Font("Segoe UI", Font.BOLD, 14)), // Larger title font
                BorderFactory.createEmptyBorder(15, 15, 15, 15))); // Wider padding
        resultsPanel.add(statusLabel);
        resultsPanel.add(meanLabel);
        resultsPanel.add(stdDevLabel);

        // Add all panels to the content panel
        JPanel topPanel = new JPanel(new BorderLayout(0, 15)); // Added gap between components
        topPanel.add(inputPanel, BorderLayout.NORTH);
        topPanel.add(scrollPane, BorderLayout.CENTER);
        topPanel.add(buttonPanel, BorderLayout.SOUTH);

        contentPanel.add(topPanel, BorderLayout.CENTER);
        contentPanel.add(resultsPanel, BorderLayout.SOUTH);

        add(contentPanel, BorderLayout.CENTER);
    }

    private void setupListeners() {
        // Add value button
        addButton.addActionListener(e -> addValue());

        // Allow pressing Enter to add value
        inputField.addActionListener(e -> addValue());

        // Calculate button
        calculateButton.addActionListener(e -> calculateStatistics());

        // Clear button
        clearButton.addActionListener(e -> {
            dataList.clear();
            dataListArea.setText("");
            resetResults();
            statusLabel.setText("Status: Ready");
        });
    }

    private void addValue() {
        try {
            String input = inputField.getText().trim();
            if (!input.isEmpty()) {
                double value = Double.parseDouble(input);
                dataList.add(value);
                updateDataListDisplay();
                inputField.setText("");
                statusLabel.setText("Status: Added value " + df.format(value));
            }
        } catch (NumberFormatException ex) {
            statusLabel.setText("Status: Error - Please enter a valid number");
        }
        inputField.requestFocus();
    }

    private void updateDataListDisplay() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < dataList.size(); i++) {
            sb.append(i + 1).append(". ").append(df.format(dataList.get(i))).append("\n");
        }
        dataListArea.setText(sb.toString());
    }

    private void resetResults() {
        meanLabel.setText("Mean: —");
        stdDevLabel.setText("Standard Deviation: —");
    }

    private void calculateStatistics() {
        if (dataList.isEmpty()) {
            statusLabel.setText("Status: Error - No data to calculate");
            return;
        }

        statusLabel.setText("Status: Connecting to server...");

        // Convert ArrayList to array
        double[] data = new double[dataList.size()];
        for (int i = 0; i < dataList.size(); i++) {
            data[i] = dataList.get(i);
        }

        // Use SwingWorker to avoid freezing UI during network operations
        SwingWorker<StatisticsResult, Void> worker = new SwingWorker<>() {
            @Override
            protected StatisticsResult doInBackground() throws Exception {
                try (
                        Socket socket = new Socket(SERVER_ADDRESS, PORT);
                        ObjectOutputStream outputStream = new ObjectOutputStream(socket.getOutputStream());
                        ObjectInputStream inputStream = new ObjectInputStream(socket.getInputStream());) {
                    // Send data to server
                    outputStream.writeObject(data);

                    // Receive results from server
                    return (StatisticsResult) inputStream.readObject();
                }
            }

            @Override
            protected void done() {
                try {
                    StatisticsResult result = get();
                    if ("Success".equals(result.getStatus())) {
                        statusLabel.setText("Status: Calculation complete");
                        meanLabel.setText("Mean: " + df.format(result.getMean()));
                        stdDevLabel.setText("Standard Deviation: " + df.format(result.getStandardDeviation()));
                    } else {
                        statusLabel.setText("Status: " + result.getStatus());
                    }
                } catch (Exception e) {
                    statusLabel.setText("Status: Error - " + e.getMessage());
                    resetResults();
                }
            }
        };

        worker.execute();
    }

    public static void main(String[] args) {
        // Set look and feel to system default
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }

        // Start the GUI
        SwingUtilities.invokeLater(() -> new Client());
    }
}

// Class to receive results from server
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