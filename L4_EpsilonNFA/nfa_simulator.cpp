#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <string>

// Structure to represent the NFA
struct NFA
{
    std::set<std::string> states;
    std::set<char> alphabet;
    std::map<std::pair<std::string, char>, std::set<std::string>> transitions;
    std::string initial_state;
    std::set<std::string> final_states;
};

// Global variables
NFA nfa;
GtkWidget *text_view_output;
GtkWidget *entry_input_string;
GtkTextBuffer *buffer;

// Function to compute epsilon closure of a set of states
std::set<std::string> epsilon_closure(const std::set<std::string> &states)
{
    std::set<std::string> closure = states;
    std::queue<std::string> queue;

    // Initialize queue with all states
    for (const auto &state : states)
    {
        queue.push(state);
    }

    // Process queue
    while (!queue.empty())
    {
        std::string current = queue.front();
        queue.pop();

        // Check for epsilon transitions
        auto transition_key = std::make_pair(current, 'e');
        if (nfa.transitions.find(transition_key) != nfa.transitions.end())
        {
            for (const auto &next_state : nfa.transitions[transition_key])
            {
                if (closure.find(next_state) == closure.end())
                {
                    closure.insert(next_state);
                    queue.push(next_state);
                }
            }
        }
    }

    return closure;
}

// Function to move from a set of states on a given input symbol
std::set<std::string> move(const std::set<std::string> &states, char symbol)
{
    std::set<std::string> result;

    for (const auto &state : states)
    {
        auto transition_key = std::make_pair(state, symbol);
        if (nfa.transitions.find(transition_key) != nfa.transitions.end())
        {
            for (const auto &next_state : nfa.transitions[transition_key])
            {
                result.insert(next_state);
            }
        }
    }

    return result;
}

// Path tracking structure
struct PathNode
{
    std::string state;
    std::string path;
    int input_pos;

    PathNode(const std::string &s, const std::string &p, int pos)
        : state(s), path(p), input_pos(pos) {}
};

// Function to simulate the NFA and track all paths
bool simulate_nfa(const std::string &input)
{
    std::vector<std::string> all_paths;
    bool accepted = false;

    // Initialize with epsilon closure of initial state
    std::set<std::string> initial_closure = epsilon_closure({nfa.initial_state});

    // Queue for BFS traversal with path tracking
    std::queue<PathNode> path_queue;

    // Add initial states with their paths
    for (const auto &state : initial_closure)
    {
        path_queue.push(PathNode(state, nfa.initial_state + " -e-> " + state, 0));
    }

    // Also add the initial state itself
    path_queue.push(PathNode(nfa.initial_state, nfa.initial_state, 0));

    // Process all possible paths
    while (!path_queue.empty())
    {
        PathNode current = path_queue.front();
        path_queue.pop();

        // If we've processed the entire input
        if (current.input_pos >= input.length())
        {
            // Check if we're in a final state
            if (nfa.final_states.find(current.state) != nfa.final_states.end())
            {
                all_paths.push_back(current.path + " (ACCEPTED)");
                accepted = true;
            }
            else
            {
                all_paths.push_back(current.path + " (REJECTED)");
            }
            continue;
        }

        char symbol = input[current.input_pos];

        // Get next states on the current symbol
        std::set<std::string> next_states = move({current.state}, symbol);

        // For each direct transition
        for (const auto &next : next_states)
        {
            std::string new_path = current.path + " -" + symbol + "-> " + next;

            // Get epsilon closure of the next state
            std::set<std::string> next_closure = epsilon_closure({next});

            // Add the direct transition
            path_queue.push(PathNode(next, new_path, current.input_pos + 1));

            // Add epsilon transitions from the next state
            for (const auto &eps_state : next_closure)
            {
                if (eps_state != next)
                {
                    path_queue.push(PathNode(
                        eps_state,
                        new_path + " -e-> " + eps_state,
                        current.input_pos + 1));
                }
            }
        }
    }

    // Display all paths
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);

    gtk_text_buffer_insert(buffer, &iter, "Paths for input: ", -1);
    gtk_text_buffer_insert(buffer, &iter, input.c_str(), -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);

    for (const auto &path : all_paths)
    {
        gtk_text_buffer_insert(buffer, &iter, path.c_str(), -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    }

    gtk_text_buffer_insert(buffer, &iter, "\nResult: ", -1);
    if (accepted)
    {
        gtk_text_buffer_insert(buffer, &iter, "ACCEPTED\n\n", -1);
    }
    else
    {
        gtk_text_buffer_insert(buffer, &iter, "REJECTED\n\n", -1);
    }

    return accepted;
}

// Function to parse the NFA definition file
bool parse_nfa_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    std::string line;

    // Read initial state
    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        ss >> token; // Read "initial"
        ss >> token; // Read "state"
        ss >> nfa.initial_state;
    }

    // Read final states
    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        ss >> token; // Read "final"
        ss >> token; // Read "is"

        while (ss >> token)
        {
            // Remove commas if present
            if (!token.empty() && token.back() == ',')
            {
                token.pop_back();
            }
            nfa.final_states.insert(token);
        }
    }

    // Read all states
    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        ss >> token; // Read "all"
        ss >> token; // Read "states"

        while (ss >> token)
        {
            // Remove commas if present
            if (!token.empty() && token.back() == ',')
            {
                token.pop_back();
            }
            nfa.states.insert(token);
        }
    }

    // Read transitions
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string from_state, to_state;
        char symbol;

        ss >> from_state;
        ss >> symbol;
        ss >> to_state;

        // Handle epsilon as 'e'
        nfa.transitions[std::make_pair(from_state, symbol)].insert(to_state);

        // Add to alphabet if not epsilon
        if (symbol != 'e')
        {
            nfa.alphabet.insert(symbol);
        }
    }

    file.close();
    return true;
}

// Callback for the "Test" button
static void on_test_button_clicked(GtkWidget *widget, gpointer data)
{
    const gchar *input_text = gtk_entry_get_text(GTK_ENTRY(entry_input_string));
    std::string input(input_text);

    // Test the input string
    simulate_nfa(input);

    // Scroll to the bottom of the text view
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view_output), &iter, 0.0, TRUE, 0.0, 1.0);
}

// Callback for the "Load NFA" button
static void on_load_button_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Open NFA Definition File",
                                         GTK_WINDOW(data),
                                         action,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        // Clear previous NFA
        nfa = NFA();

        // Parse the new NFA definition
        if (parse_nfa_file(filename))
        {
            GtkTextIter iter;
            gtk_text_buffer_get_start_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, "NFA loaded successfully!\n\n", -1);

            // Display NFA details
            std::stringstream ss;

            ss << "Initial State: " << nfa.initial_state << "\n";

            ss << "Final States: ";
            for (const auto &state : nfa.final_states)
            {
                ss << state << " ";
            }
            ss << "\n";

            ss << "All States: ";
            for (const auto &state : nfa.states)
            {
                ss << state << " ";
            }
            ss << "\n";

            ss << "Alphabet: ";
            for (const auto &symbol : nfa.alphabet)
            {
                ss << symbol << " ";
            }
            ss << "e (epsilon)\n";

            ss << "Transitions:\n";
            for (const auto &transition : nfa.transitions)
            {
                std::string from_state = transition.first.first;
                char symbol = transition.first.second;

                for (const auto &to_state : transition.second)
                {
                    ss << "  " << from_state << " -" << symbol << "-> " << to_state << "\n";
                }
            }
            ss << "\n";

            gtk_text_buffer_insert(buffer, &iter, ss.str().c_str(), -1);
        }
        else
        {
            GtkTextIter iter;
            gtk_text_buffer_get_start_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, "Failed to load NFA!\n", -1);
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Callback for the "Clear" button
static void on_clear_button_clicked(GtkWidget *widget, gpointer data)
{
    gtk_text_buffer_set_text(buffer, "", -1);
    gtk_entry_set_text(GTK_ENTRY(entry_input_string), "");
}

// Activate function
static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label_input;
    GtkWidget *scrolled_window;
    GtkWidget *button_test;
    GtkWidget *button_load;
    GtkWidget *button_clear;

    // Create a new window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "ε-NFA Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Create a grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    // Create input label and entry
    label_input = gtk_label_new("Input String:");
    gtk_grid_attach(GTK_GRID(grid), label_input, 0, 0, 1, 1);

    entry_input_string = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_input_string, 1, 0, 3, 1);

    // Create the test button
    button_test = gtk_button_new_with_label("Test String");
    gtk_grid_attach(GTK_GRID(grid), button_test, 4, 0, 1, 1);
    g_signal_connect(button_test, "clicked", G_CALLBACK(on_test_button_clicked), NULL);

    // Create the load button
    button_load = gtk_button_new_with_label("Load NFA");
    gtk_grid_attach(GTK_GRID(grid), button_load, 5, 0, 1, 1);
    g_signal_connect(button_load, "clicked", G_CALLBACK(on_load_button_clicked), window);

    // Create the clear button
    button_clear = gtk_button_new_with_label("Clear");
    gtk_grid_attach(GTK_GRID(grid), button_clear, 6, 0, 1, 1);
    g_signal_connect(button_clear, "clicked", G_CALLBACK(on_clear_button_clicked), NULL);

    // Create a scrolled window
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window, 0, 1, 7, 1);

    // Create a text view for output
    text_view_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_output), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view_output);

    // Get the buffer for the text view
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_output));

    // Show all widgets
    gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.example.nfa_simulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
