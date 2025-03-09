import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.util.List;
import javax.swing.*;

public class GrahamScanConvexHull extends JFrame {
    private List<Point> points = new ArrayList<>();
    private List<Point> convexHull = new ArrayList<>();
    private List<List<Point>> steps = new ArrayList<>();
    private List<Point> currentVertices = new ArrayList<>(); // Track current vertex for each step
    private int currentStep = -1;
    private boolean showSteps = false;
    private JButton computeButton;
    private JButton resetButton;
    private JButton nextStepButton;
    private JButton prevStepButton;
    private JCheckBox showStepsCheckBox;
    private JLabel statusLabel;

    public GrahamScanConvexHull() {
        setTitle("Graham Scan Convex Hull");
        setSize(800, 600);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        // Create panel for buttons
        JPanel controlPanel = new JPanel();
        computeButton = new JButton("Compute Convex Hull");
        resetButton = new JButton("Reset");
        nextStepButton = new JButton("Next Step");
        prevStepButton = new JButton("Previous Step");
        showStepsCheckBox = new JCheckBox("Show Steps");
        statusLabel = new JLabel("Click to add points");

        nextStepButton.setEnabled(false);
        prevStepButton.setEnabled(false);

        controlPanel.add(computeButton);
        controlPanel.add(resetButton);
        controlPanel.add(prevStepButton);
        controlPanel.add(nextStepButton);
        controlPanel.add(showStepsCheckBox);
        controlPanel.add(statusLabel);

        add(controlPanel, BorderLayout.SOUTH);

        DrawingPanel drawingPanel = new DrawingPanel();
        add(drawingPanel, BorderLayout.CENTER);

        // Add action listeners
        computeButton.addActionListener(e -> {
            if (points.size() < 3) {
                JOptionPane.showMessageDialog(this,
                        "Need at least 3 points to compute convex hull",
                        "Not enough points", JOptionPane.WARNING_MESSAGE);
                return;
            }

            computeConvexHull();
            showSteps = showStepsCheckBox.isSelected();

            if (showSteps) {
                currentStep = 0;
                nextStepButton.setEnabled(true);
                prevStepButton.setEnabled(false);
                statusLabel.setText("Step 1 of " + steps.size());
            } else {
                currentStep = steps.size() - 1;
                nextStepButton.setEnabled(false);
                prevStepButton.setEnabled(false);
                statusLabel.setText("Convex Hull Computed");
            }

            repaint();
        });

        resetButton.addActionListener(e -> {
            points.clear();
            convexHull.clear();
            steps.clear();
            currentVertices.clear();
            currentStep = -1;
            nextStepButton.setEnabled(false);
            prevStepButton.setEnabled(false);
            statusLabel.setText("Click to add points");
            repaint();
        });

        nextStepButton.addActionListener(e -> {
            if (currentStep < steps.size() - 1) {
                currentStep++;
                prevStepButton.setEnabled(true);
                if (currentStep == steps.size() - 1) {
                    nextStepButton.setEnabled(false);
                }
                statusLabel.setText("Step " + (currentStep + 1) + " of " + steps.size());
                repaint();
            }
        });

        prevStepButton.addActionListener(e -> {
            if (currentStep > 0) {
                currentStep--;
                nextStepButton.setEnabled(true);
                if (currentStep == 0) {
                    prevStepButton.setEnabled(false);
                }
                statusLabel.setText("Step " + (currentStep + 1) + " of " + steps.size());
                repaint();
            }
        });

        showStepsCheckBox.addActionListener(e -> {
            showSteps = showStepsCheckBox.isSelected();
            if (steps.size() > 0) {
                if (showSteps) {
                    currentStep = 0;
                    nextStepButton.setEnabled(true);
                    prevStepButton.setEnabled(false);
                    statusLabel.setText("Step 1 of " + steps.size());
                } else {
                    currentStep = steps.size() - 1;
                    nextStepButton.setEnabled(false);
                    prevStepButton.setEnabled(false);
                    statusLabel.setText("Convex Hull Computed");
                }
                repaint();
            }
        });
    }

    private Point findLowestPoint() {
        Point lowest = points.get(0);
        for (Point p : points) {
            if (p.y > lowest.y || (p.y == lowest.y && p.x < lowest.x)) {
                lowest = p;
            }
        }
        return lowest;
    }

    private int orientation(Point p, Point q, Point r) {
        int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
        if (val == 0)
            return 0; // collinear
        return (val > 0) ? 1 : 2; // clockwise or counterclockwise
    }

    private int distanceSquared(Point p1, Point p2) {
        return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
    }

    private void computeConvexHull() {
        steps.clear();
        convexHull.clear();
        currentVertices.clear();

        if (points.size() < 3)
            return;

        // Find the lowest point
        Point lowestPoint = findLowestPoint();

        // Sort points based on polar angle with respect to the lowest point
        Point finalLowestPoint = lowestPoint;
        List<Point> sortedPoints = new ArrayList<>(points);
        sortedPoints.sort((p1, p2) -> {
            int o = orientation(finalLowestPoint, p1, p2);
            if (o == 0) {
                return distanceSquared(finalLowestPoint, p1) - distanceSquared(finalLowestPoint, p2);
            }
            return (o == 2) ? -1 : 1;
        });

        // Initialize convex hull with first 2 points
        Stack<Point> stack = new Stack<>();
        stack.push(sortedPoints.get(0));
        stack.push(sortedPoints.get(1));

        // First step with first 2 points
        steps.add(new ArrayList<>(stack));
        currentVertices.add(sortedPoints.get(2)); // Next vertex to be considered

        // Process remaining points
        for (int i = 2; i < sortedPoints.size(); i++) {
            Point currentVertex = sortedPoints.get(i);

            // Before processing this vertex, save its state
            List<Point> beforeHull = new ArrayList<>(stack);
            steps.add(new ArrayList<>(beforeHull));
            currentVertices.add(currentVertex);

            // Remove points that make non-left turns
            while (stack.size() > 1 && orientation(stack.get(stack.size() - 2),
                    stack.peek(), currentVertex) != 2) {
                stack.pop();
            }

            // Add current point to hull
            stack.push(currentVertex);

            // Prepare next vertex to consider (if any)
            Point nextVertex = (i < sortedPoints.size() - 1) ? sortedPoints.get(i + 1) : null;

            // After processing, save the state
            List<Point> afterHull = new ArrayList<>(stack);
            steps.add(new ArrayList<>(afterHull));
            currentVertices.add(nextVertex);
        }

        // Transfer stack to convexHull
        convexHull = new ArrayList<>(stack);

        // Final hull - no vertex highlighted
        steps.add(new ArrayList<>(convexHull));
        currentVertices.add(null);
    }

    class DrawingPanel extends JPanel {
        public DrawingPanel() {
            setBackground(Color.WHITE);

            // Mouse listener to add points
            addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    points.add(new Point(e.getX(), e.getY()));

                    // Automatically update convex hull if already computed
                    if (!convexHull.isEmpty()) {
                        computeConvexHull();
                        if (showSteps) {
                            currentStep = 0;
                            statusLabel.setText("Step 1 of " + steps.size());
                        } else {
                            currentStep = steps.size() - 1;
                            statusLabel.setText("Convex Hull Updated");
                        }
                    } else {
                        statusLabel.setText("Points: " + points.size());
                    }

                    repaint();
                }
            });
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Graphics2D g2d = (Graphics2D) g;
            g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

            // Draw all points
            g2d.setColor(Color.BLACK);
            for (Point p : points) {
                g2d.fillOval(p.x - 4, p.y - 4, 8, 8);
            }

            // Draw the convex hull steps or final hull
            if (currentStep >= 0 && currentStep < steps.size()) {
                List<Point> currentHull = steps.get(currentStep);

                // Draw connections between points
                g2d.setColor(Color.RED);
                g2d.setStroke(new BasicStroke(2));

                for (int i = 0; i < currentHull.size(); i++) {
                    Point current = currentHull.get(i);
                    Point next = currentHull.get((i + 1) % currentHull.size());
                    g2d.drawLine(current.x, current.y, next.x, next.y);
                }

                // Highlight hull points
                g2d.setColor(Color.RED);
                for (Point p : currentHull) {
                    g2d.fillOval(p.x - 6, p.y - 6, 12, 12);
                    g2d.setColor(Color.WHITE);
                    g2d.fillOval(p.x - 4, p.y - 4, 8, 8);
                    g2d.setColor(Color.RED);
                }

                // Highlight current vertex with yellow
                if (currentStep < currentVertices.size() && currentVertices.get(currentStep) != null) {
                    Point currentVertex = currentVertices.get(currentStep);
                    g2d.setColor(Color.YELLOW);
                    g2d.fillOval(currentVertex.x - 7, currentVertex.y - 7, 14, 14);
                }
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            GrahamScanConvexHull frame = new GrahamScanConvexHull();
            frame.setVisible(true);
        });
    }
}