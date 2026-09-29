#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

// looks for name in every directory of the search path, in order
string find_executable(const string &name)
{
    for (const string &dir : search_path) {
        string full = dir + "/" + name;
        if (access(full.c_str(), X_OK) == 0)
            return full;
    }
    return "";
}

pid_t launch(const vector<string> &args)
{
    string prog = find_executable(args[0]);
    if (prog.empty()) {
        print_error();
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        print_error();
        return -1;
    }

    if (pid == 0) {
        // execv needs a NULL-terminated char* array; the pointers point into
        // args, which stays alive in the child until exec replaces the image
        vector<char *> argv;
        for (const string &a : args)
            argv.push_back(const_cast<char *>(a.c_str()));
        argv.push_back(nullptr);

        execv(prog.c_str(), argv.data());
        // only reached if execv failed
        print_error();
        exit(1);
    }

    return pid;
}

void process_line(const string &line)
{
    vector<string> args = parse_args(line);

    if (args.empty())
        return;

    if (run_builtin(args))
        return;

    pid_t pid = launch(args);
    if (pid > 0)
        waitpid(pid, nullptr, 0);
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
