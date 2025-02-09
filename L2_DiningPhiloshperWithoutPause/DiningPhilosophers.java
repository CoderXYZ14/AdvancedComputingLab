import java.awt.*;
import java.util.concurrent.Semaphore;
import javax.swing.*;

// State enum needs to be accessible by all classes
enum PhilosopherState {
    THINKING, HUNGRY, EATING
}

public class DiningPhilosophers extends JFrame {
    private static final int NUM_PHILOSOPHERS = 5;
    private final Semaphore[] chopsticks = new Semaphore[NUM_PHILOSOPHERS];
    private final Philosopher[] philosophers = new Philosopher[NUM_PHILOSOPHERS];
    private final PhilosopherPanel philosopherPanel;
    private final JTextArea statusArea;
    private final JButton startButton;
    private volatile boolean isPaused = true;

    public DiningPhilosophers() {
        setTitle("Dining Philosophers Visualization");
        setSize(800, 800);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        // Initialize chopsticks
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            chopsticks[i] = new Semaphore(1);
        }

        // Create control panel
        JPanel controlPanel = new JPanel();
        startButton = new JButton("Start");
        startButton.addActionListener(e -> toggleSimulation());
        controlPanel.add(startButton);

        // Create legend panel
        JPanel legendPanel = new JPanel();
        legendPanel.setBorder(BorderFactory.createTitledBorder("Legend"));
        legendPanel.setLayout(new GridLayout(3, 1));
        addLegendItem(legendPanel, "Thinking - Blue", Color.BLUE);
        addLegendItem(legendPanel, "Hungry - Red", Color.RED);
        addLegendItem(legendPanel, "Eating - Green", Color.GREEN);

        // Create status area
        statusArea = new JTextArea(10, 40);
        statusArea.setEditable(false);
        JScrollPane scrollPane = new JScrollPane(statusArea);
        scrollPane.setBorder(BorderFactory.createTitledBorder("Status Log"));

        // Create top panel for controls and legend
        JPanel topPanel = new JPanel(new FlowLayout());
        topPanel.add(controlPanel);
        topPanel.add(legendPanel);

        // Add components to frame
        add(topPanel, BorderLayout.NORTH);
        philosopherPanel = new PhilosopherPanel();
        add(philosopherPanel, BorderLayout.CENTER);
        add(scrollPane, BorderLayout.SOUTH);

        // Initialize philosophers
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            philosophers[i] = new Philosopher(i);
            philosophers[i].start(); // Start threads immediately but they'll wait due to isPaused
        }
    }

    private void addLegendItem(JPanel panel, String text, Color color) {
        JPanel item = new JPanel(new FlowLayout(FlowLayout.LEFT));
        JPanel colorBox = new JPanel();
        colorBox.setBackground(color);
        colorBox.setPreferredSize(new Dimension(20, 20));
        item.add(colorBox);
        item.add(new JLabel(text));
        panel.add(item);
    }

    private void toggleSimulation() {
        isPaused = !isPaused;
        startButton.setText(isPaused ? "Start" : "Stop");
        logStatus(isPaused ? "Simulation paused\n" : "Simulation resumed\n");

        // Wake up all philosophers to either continue or check pause state
        synchronized (DiningPhilosophers.this) {
            notifyAll();
        }
    }

    private void logStatus(String message) {
        SwingUtilities.invokeLater(() -> {
            statusArea.append(message);
            statusArea.setCaretPosition(statusArea.getDocument().getLength());
        });
    }

    private class Philosopher extends Thread {
        private final int id;
        private PhilosopherState currentState;

        public Philosopher(int id) {
            this.id = id;
            this.currentState = PhilosopherState.THINKING;
        }

        public PhilosopherState getCurrentState() {
            return currentState;
        }

        private void checkPaused() throws InterruptedException {
            synchronized (DiningPhilosophers.this) {
                while (isPaused) {
                    DiningPhilosophers.this.wait();
                }
            }
        }

        @Override
        public void run() {
            try {
                while (!Thread.interrupted()) {
                    checkPaused();
                    think();
                    checkPaused();
                    pickUpChopsticks();
                    checkPaused();
                    eat();
                    checkPaused();
                    putDownChopsticks();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        private void think() throws InterruptedException {
            currentState = PhilosopherState.THINKING;
            logStatus("Philosopher " + id + " is thinking\n");
            philosopherPanel.repaint();
            Thread.sleep((long) (Math.random() * 5000 + 5000));
        }

        private void pickUpChopsticks() throws InterruptedException {
            currentState = PhilosopherState.HUNGRY;
            logStatus("Philosopher " + id + " is hungry and waiting for chopsticks\n");
            philosopherPanel.repaint();

            chopsticks[id].acquire();
            logStatus("Philosopher " + id + " picked up left chopstick\n");
            Thread.sleep(2000);

            chopsticks[(id + 1) % NUM_PHILOSOPHERS].acquire();
            logStatus("Philosopher " + id + " picked up right chopstick\n");
        }

        private void eat() throws InterruptedException {
            currentState = PhilosopherState.EATING;
            logStatus("Philosopher " + id + " is eating\n");
            philosopherPanel.repaint();
            Thread.sleep((long) (Math.random() * 5000 + 5000));
        }

        private void putDownChopsticks() {
            chopsticks[id].release();
            chopsticks[(id + 1) % NUM_PHILOSOPHERS].release();
            logStatus("Philosopher " + id + " put down chopsticks\n");
        }
    }

    private class PhilosopherPanel extends JPanel {
        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Graphics2D g2d = (Graphics2D) g;
            g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

            int centerX = getWidth() / 2;
            int centerY = getHeight() / 2;
            int radius = Math.min(centerX, centerY) - 100;

            // Draw table
            g2d.setColor(Color.GRAY);
            g2d.fillOval(centerX - radius, centerY - radius, radius * 2, radius * 2);

            // Draw philosophers and chopsticks
            for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
                double angle = i * 2 * Math.PI / NUM_PHILOSOPHERS;
                int x = (int) (centerX + radius * Math.cos(angle));
                int y = (int) (centerY + radius * Math.sin(angle));

                // Draw philosopher
                if (philosophers[i] != null) {
                    switch (philosophers[i].getCurrentState()) {
                        case THINKING:
                            g2d.setColor(Color.BLUE);
                            break;
                        case HUNGRY:
                            g2d.setColor(Color.RED);
                            break;
                        case EATING:
                            g2d.setColor(Color.GREEN);
                            break;
                    }
                } else {
                    g2d.setColor(Color.GRAY);
                }

                g2d.fillOval(x - 20, y - 20, 40, 40);
                g2d.setColor(Color.BLACK);
                g2d.drawString("P" + i, x - 5, y + 5);

                // Draw chopstick
                double stickAngle = (i + 0.5) * 2 * Math.PI / NUM_PHILOSOPHERS;
                int x1 = (int) (centerX + (radius - 40) * Math.cos(stickAngle));
                int y1 = (int) (centerY + (radius - 40) * Math.sin(stickAngle));
                int x2 = (int) (centerX + (radius + 40) * Math.cos(stickAngle));
                int y2 = (int) (centerY + (radius + 40) * Math.sin(stickAngle));
                g2d.setColor(Color.BLACK);
                g2d.drawLine(x1, y1, x2, y2);
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            DiningPhilosophers frame = new DiningPhilosophers();
            frame.setVisible(true);
        });
    }
}
