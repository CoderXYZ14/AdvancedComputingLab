#include <gtk/gtk.h>
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <sstream>

using namespace std;

class MaxFlowSolver
{
private:
    int vertices;
    vector<vector<int>> graph;
    vector<vector<int>> residualGraph;
    vector<string> steps;

    bool breadthFirstSearch(int source, int sink, vector<int> &parent)
    {
        vector<bool> visited(vertices, false);
        queue<int> q;

        q.push(source);
        visited[source] = true;
        parent[source] = -1;

        while (!q.empty())
        {
            int u = q.front();
            q.pop();

            for (int v = 0; v < vertices; v++)
            {
                if (!visited[v] && residualGraph[u][v] > 0)
                {
                    q.push(v);
                    parent[v] = u;
                    visited[v] = true;
                }
            }
        }

        return visited[sink];
    }

public:
    MaxFlowSolver(int v) : vertices(v), graph(v, vector<int>(v, 0)),
                           residualGraph(v, vector<int>(v, 0)) {}

    void addEdge(int u, int v, int capacity)
    {
        graph[u][v] = capacity;
        residualGraph[u][v] = capacity;
    }

    int fordFulkerson(int source, int sink)
    {
        steps.clear(); // why important
        vector<int> parent(vertices);
        int maxFlow = 0;
        stringstream stepStream;

        stepStream << "Initial Graph:\n";
        for (int i = 0; i < vertices; i++)
        {
            for (int j = 0; j < vertices; j++)
            {
                if (graph[i][j] > 0)
                {
                    stepStream << "Edge (" << i << "->" << j << "): "
                               << graph[i][j] << "\n";
                }
            }
        }
        steps.push_back(stepStream.str()); // why storing in steps
        stepStream.str("");

        while (breadthFirstSearch(source, sink, parent))
        {
            int pathFlow = numeric_limits<int>::max(); // change to int_max

            // to find bottleneck capacity
            for (int v = sink; v != source; v = parent[v])
            {
                int u = parent[v];
                pathFlow = min(pathFlow, residualGraph[u][v]);
            }

            for (int v = sink; v != source; v = parent[v])
            {
                int u = parent[v];
                residualGraph[u][v] -= pathFlow;
                residualGraph[v][u] += pathFlow; // remove it
            }

            stepStream << "Augmenting Path Found. Path Flow: " << pathFlow << "\n";
            for (int v = sink; v != source; v = parent[v])
            {
                int u = parent[v];
                stepStream << u << " -> " << v << "\n";
            }
            steps.push_back(stepStream.str());
            stepStream.str("");

            maxFlow += pathFlow;
        }

        stepStream << "Maximum Flow: " << maxFlow << "\n";
        steps.push_back(stepStream.str());

        return maxFlow;
    }

    const vector<string> &getSteps()
    {
        return steps;
    }
};

// GTK GUI Components
GtkWidget *window, *box, *textview, *runButton, *sourceEntry, *sinkEntry, *edgeEntry;
MaxFlowSolver *solver = nullptr;

static void display_steps(GtkWidget *widget, gpointer data)
{
    GtkTextBuffer *buffer; // for text view
    GtkTextIter end;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_get_end_iter(buffer, &end);

    if (solver)
    {
        const vector<string> &steps = solver->getSteps();
        for (const auto &step : steps)
        {
            gtk_text_buffer_insert(buffer, &end, step.c_str(), -1);
            gtk_text_buffer_insert(buffer, &end, "\n", -1);
            gtk_text_buffer_get_end_iter(buffer, &end);
        }
    }
}

static void run_max_flow(GtkWidget *widget, gpointer data)
{
    // Clear previous results
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, "", -1);

    // Get user inputs
    const gchar *sourceText = gtk_entry_get_text(GTK_ENTRY(sourceEntry));
    const gchar *sinkText = gtk_entry_get_text(GTK_ENTRY(sinkEntry));
    const gchar *edgeText = gtk_entry_get_text(GTK_ENTRY(edgeEntry));

    int source = stoi(sourceText);
    int sink = stoi(sinkText);
    int numVertices = source > sink ? source + 1 : sink + 1;

    solver = new MaxFlowSolver(numVertices);

    // Parse and add edges
    istringstream edgeStream(edgeText);
    int u, v, capacity;
    while (edgeStream >> u >> v >> capacity)
    {
        solver->addEdge(u, v, capacity);
    }

    int maxFlow = solver->fordFulkerson(source, sink);
    display_steps(widget, data);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Max Flow Algorithm");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Source node input
    GtkWidget *sourceLabel = gtk_label_new("Source Node:");
    sourceEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(sourceEntry), "Enter source node");
    gtk_box_pack_start(GTK_BOX(box), sourceLabel, FALSE, FALSE, 0);

    // Sink node input
    GtkWidget *sinkLabel = gtk_label_new("Sink Node:");
    sinkEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(sinkEntry), "Enter sink node");
    gtk_box_pack_start(GTK_BOX(box), sinkLabel, FALSE, FALSE, 0);

    // Edge input
    GtkWidget *edgeLabel = gtk_label_new("Edges (format: u v capacity):");
    edgeEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(edgeEntry), "0 1 16 0 2 13 1 3 12...");
    gtk_box_pack_start(GTK_BOX(box), edgeLabel, FALSE, FALSE, 0);

    // Run button
    runButton = gtk_button_new_with_label("Run Max Flow");
    g_signal_connect(runButton, "clicked", G_CALLBACK(run_max_flow), NULL);
    gtk_box_pack_start(GTK_BOX(box), runButton, FALSE, FALSE, 0);

    // Text view for results
    textview = gtk_text_view_new();
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), textview);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.example.maxflow", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}