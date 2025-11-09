#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

unsigned char factorial(unsigned int n) {
    unsigned int r = 1;
    for (unsigned int i = 2; i <= n; ++i) r = (r * i) % 256;
    return static_cast<unsigned char>(r);
}

bool isSymbol(char c) {
    return c == '+' || c == '-' || c == '<' || c == '>' || c == '.' || c == '!' || isspace(c);
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2) {
        cerr << "Usage: glass <file.gl>\n";
        return 1;
    }

    string filename = argv[1];
    if (filename.size() < 3 || filename.substr(filename.size() - 3) != ".gl") {
        cerr << "Error: must use .gl extension\n";
        return 1;
    }

    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file\n";
        return 1;
    }

    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    vector<unsigned char> tape(30000, 0);
    size_t ptr = 0;
    int pendingPlus = 0, pendingMinus = 0;

    auto flushPending = [&]() {
        tape[ptr] = (tape[ptr] + pendingPlus - pendingMinus + 256 * 10) % 256;
        pendingPlus = pendingMinus = 0;
    };

    for (size_t i = 0; i < code.size();) {
        char c = code[i];
        if (!isSymbol(c)) { i++; continue; }

        if (isspace(c)) { flushPending(); i++; continue; }

        // normal arithmetic
        if (c == '+') {
            // check for +- sequence
            if (i + 1 < code.size() && code[i + 1] == '-') {
                flushPending();
                bool move = false;
                size_t j = i + 2;
                if (j < code.size() && code[j] == '+') {
                    move = true;
                    j++;
                }

                int distance = 0;
                char dir = 0;
                while (j < code.size() && (code[j] == '<' || code[j] == '>')) {
                    dir = code[j];
                    distance++;
                    j++;
                }

                if (distance > 0) {
                    long target = static_cast<long>(ptr) + (dir == '<' ? -distance : distance);
                    if (target < 0) target = 0;
                    if (static_cast<size_t>(target) >= tape.size()) tape.resize(target + 1, 0);

                    tape[target] = tape[ptr];
                    if (move) tape[ptr] = 0;

                    ptr = static_cast<size_t>(target);
                }

                i = j;
                continue;
            } else {
                pendingPlus++;
                i++;
                continue;
            }
        }

        if (c == '-') { pendingMinus++; i++; continue; }

        // factorials
        if (c == '!') {
            if (pendingPlus || pendingMinus) {
                int n = pendingPlus ? pendingPlus : pendingMinus;
                unsigned char f = factorial(n);
                if (pendingPlus) tape[ptr] = (tape[ptr] + f) % 256;
                else tape[ptr] = (tape[ptr] - f + 256) % 256;
                pendingPlus = pendingMinus = 0;
            } else {
                tape[ptr] = factorial(tape[ptr]);
            }
            i++;
            continue;
        }

        // pointer movement
        if (c == '<') { flushPending(); if (ptr > 0) ptr--; i++; continue; }
        if (c == '>') { flushPending(); ptr++; if (ptr >= tape.size()) tape.resize(ptr + 1, 0); i++; continue; }

        // output
        if (c == '.') {
            flushPending();
            cout << static_cast<char>(tape[ptr]);
            i++;
            continue;
        }

        i++;
    }

    flushPending();
    return 0;
}
