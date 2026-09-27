#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>

using namespace std;

char error_message[30] = "An error has occurred\n";

vector<string> search_path = {"/bin"};

void print_error()
{
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// operator>> skips any run of spaces/tabs, so empty tokens never appear
vector<string> parse_args(const string &line)
{
    vector<string> args;
    istringstream ss(line);
    string tok;

    while (ss >> tok)
        args.push_back(tok);
    return args;
}

// returns true if args[0] was a built-in, so the caller won't try to exec it
bool run_builtin(const vector<string> &args)
{
    const string &cmd = args[0];

    if (cmd == "exit") {
        if (args.size() != 1) {
            print_error();
            return true;
        }
        exit(0);
    }

    if (cmd == "cd") {
        if (args.size() != 2 || chdir(args[1].c_str()) != 0)
            print_error();
        return true;
    }

    if (cmd == "path") {
        // path always replaces the old list; no args means an empty path
        search_path.assign(args.begin() + 1, args.end());
        return true;
    }

    return false;
}

void process_line(const string &line)
{
    vector<string> args = parse_args(line);

    if (args.empty())
        return;

    if (run_builtin(args))
        return;

#ifdef DEBUG
    for (const string &a : args)
        cout << "[" << a << "] ";
    cout << endl;
#endif
}

int main(int argc, char *argv[])
{
    if (argc > 2) {
        print_error();
        exit(1);
    }

    ifstream file;
    bool interactive = true;

    if (argc == 2) {
        file.open(argv[1]);
        if (!file.is_open()) {
            print_error();
            exit(1);
        }
        interactive = false;
    }

    // one reference for both modes, so the loop doesn't care where input comes from
    istream &in = interactive ? cin : file;
    string line;

    while (true) {
        if (interactive)
            cout << "wish> " << flush;
        if (!getline(in, line))
            break;
        process_line(line);
    }

    return 0;
}
