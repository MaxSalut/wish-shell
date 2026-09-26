#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>

using namespace std;

char error_message[30] = "An error has occurred\n";

void print_error()
{
    write(STDERR_FILENO, error_message, strlen(error_message));
}
vector<string> parse_args(const string &line)
{
    vector<string> args;
    istringstream ss(line);
    string tok;

    while (ss >> tok)
        args.push_back(tok);
    return args;
}

void process_line(const string &line)
{
    vector<string> args = parse_args(line);

    if (args.empty())
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
