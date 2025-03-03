#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <string>

using namespace std;

struct NFA
{
    set<string> states;
    set<char> alphabet;
    map<pair<string, char>, set<string>> transitions;
    string initial_state;
    set<string> final_states;
};

NFA nfa;
GtkWidget *text_view_output;
GtkWidget *entry_input_string;
GtkTextBuffer *buffer;

set<string> epsilon_closure(const set<string> &states)
{
    set<string> closure = states;
    queue<string> queue;

    for (const auto &state : states)
    {
        queue.push(state);
    }

    while (!queue.empty())
    {
        string current = queue.front();
        queue.pop();

        auto transition_key = make_pair(current, 'e');
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

set<string> move(const set<string> &states, char symbol)
{
    set<string> result;

    for (const auto &state : states)
    {
        auto transition_key = make_pair(state, symbol);
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

struct PathNode
{
    string state;
    string path;
    int input_pos;

    PathNode(const string &s, const string &p, int pos)
        : state(s), path(p), input_pos(pos) {}
};

bool simulate_nfa(const string &input)
{
    vector<string> all_paths;
    bool accepted = false;

    set<string> initial_closure = epsilon_closure({nfa.initial_state});

    queue<PathNode> path_queue;

    for (const auto &state : initial_closure)
    {
        path_queue.push(PathNode(state, nfa.initial_state + " -e-> " + state, 0));
    }

    path_queue.push(PathNode(nfa.initial_state, nfa.initial_state, 0));

    while (!path_queue.empty())
    {
        PathNode current = path_queue.front();
        path_queue.pop();

        if (current.input_pos >= input.length())
        {
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

        set<string> next_states = move({current.state}, symbol);

        for (const auto &next : next_states)
        {
            string new_path = current.path + " -" + symbol + "-> " + next;

            set<string> next_closure = epsilon_closure({next});

            path_queue.push(PathNode(next, new_path, current.input_pos + 1));

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

bool parse_nfa_file(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return false;
    }

    string line;

    if (getline(file, line))
    {
        stringstream ss(line);
        string token;
        ss >> token;
        ss >> token;
        ss >> nfa.initial_state;
    }

    if (getline(file, line))
    {
        stringstream ss(line);
        string token;
        ss >> token;
        ss >> token;

        while (ss >> token)
        {
            if (!token.empty() && token.back() == ',')
            {
                token.pop_back();
            }
            nfa.final_states.insert(token);
        }
    }

    if (getline(file, line))
    {
        stringstream ss(line);
        string token;
        ss >> token;
        ss >> token;

        while (ss >> token)
        {
            if (!token.empty() && token.back() == ',')
            {
                token.pop_back();
            }
            nfa.states.insert(token);
        }
    }

    while (getline(file, line))
    {
        stringstream ss(line);
        string from_state, to_state;
        char symbol;

        ss >> from_state;
        ss >> symbol;
        ss >> to_state;

        nfa.transitions[make_pair(from_state, symbol)].insert(to_state);

        if (symbol != 'e')
        {
            nfa.alphabet.insert(symbol);
        }
    }

    file.close();
    return true;
}

static void on_test_button_clicked(GtkWidget *widget, gpointer data)
{
    const gchar *input_text = gtk_entry_get_text(GTK_ENTRY(entry_input_string));
    string input(input_text);

    simulate_nfa(input);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view_output), &iter, 0.0, TRUE, 0.0, 1.0);
}

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

        nfa = NFA();

        if (parse_nfa_file(filename))
        {
            GtkTextIter iter;
            gtk_text_buffer_get_start_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, "NFA loaded successfully!\n\n", -1);

            stringstream ss;

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
                string from_state = transition.first.first;
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

static void on_clear_button_clicked(GtkWidget *widget, gpointer data)
{
    gtk_text_buffer_set_text(buffer, "", -1);
    gtk_entry_set_text(GTK_ENTRY(entry_input_string), "");
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label_input;
    GtkWidget *scrolled_window;
    GtkWidget *button_test;
    GtkWidget *button_load;
    GtkWidget *button_clear;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "ε-NFA Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    label_input = gtk_label_new("Input String:");
    gtk_grid_attach(GTK_GRID(grid), label_input, 0, 0, 1, 1);

    entry_input_string = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_input_string, 1, 0, 3, 1);

    button_test = gtk_button_new_with_label("Test String");
    gtk_grid_attach(GTK_GRID(grid), button_test, 4, 0, 1, 1);
    g_signal_connect(button_test, "clicked", G_CALLBACK(on_test_button_clicked), NULL);

    button_load = gtk_button_new_with_label("Load NFA");
    gtk_grid_attach(GTK_GRID(grid), button_load, 5, 0, 1, 1);
    g_signal_connect(button_load, "clicked", G_CALLBACK(on_load_button_clicked), window);

    button_clear = gtk_button_new_with_label("Clear");
    gtk_grid_attach(GTK_GRID(grid), button_clear, 6, 0, 1, 1);
    g_signal_connect(button_clear, "clicked", G_CALLBACK(on_clear_button_clicked), NULL);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window, 0, 1, 7, 1);

    text_view_output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_output), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view_output);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_output));

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