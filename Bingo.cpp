#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace std;

// ----------------- COLOR THEME -----------------
const string RESET = "\033[0m";
const string CYAN = "\033[1;36m";
const string MAGENTA = "\033[1;35m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string RED = "\033[1;31m";
const string BLUE = "\033[1;34m";

static std::mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

// ----------------- UTILITIES -----------------
int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << CYAN << prompt << RESET;
        if (cin >> value) return value;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << RED << "Invalid input. Please enter an integer.\n" << RESET;
    }
}

// ----------------- BINGO CARD CLASS -----------------
class BingoCard {
    int rows_, cols_;
    vector<int> data_; // row-major, 0 means marked
public:
    BingoCard(int rows = 0, int cols = 0) { reset(rows, cols); }

    void reset(int rows, int cols) {
        rows_ = rows;
        cols_ = cols;
        data_.assign(rows_ * cols_, 0);
    }

    void generate() {
        int total = rows_ * cols_;
        vector<int> nums(total);
        for (int i = 0; i < total; ++i) nums[i] = i + 1;
        shuffle(nums.begin(), nums.end(), rng);
        data_ = move(nums);
    }

    void display(const string& title = "", bool showHeader = true) const {
        system("cls");
        if (showHeader) {
            cout << MAGENTA << "------------------------------------------------------------\n" << RESET;
            if (!title.empty()) cout << MAGENTA << "  " << title << RESET << "\n";
            cout << MAGENTA << "------------------------------------------------------------\n\n" << RESET;
        }
        for (int r = 0; r < rows_; ++r) {
            cout << "\t";
            for (int c = 0; c < cols_; ++c) {
                int v = at(r, c);
                if (v == 0) cout << GREEN << setw(3) << "X" << "\t" << RESET;
                else cout << YELLOW << setw(3) << v << "\t" << RESET;
            }
            cout << "\n";
        }
        cout << "\n";
    }

    int at(int r, int c) const { return data_[r * cols_ + c]; }
    int& atRef(int r, int c) { return data_[r * cols_ + c]; }

    bool contains(int num) const {
        return find(data_.begin(), data_.end(), num) != data_.end();
    }

    bool mark(int num) {
        for (auto& cell : data_) {
            if (cell == num) {
                cell = 0;
                return true;
            }
        }
        return false;
    }

    int countCompletedLines() const {
        int completed = 0;
        for (int r = 0; r < rows_; ++r) {
            bool allZero = true;
            for (int c = 0; c < cols_; ++c) {
                if (at(r, c) != 0) { allZero = false; break; }
            }
            if (allZero) ++completed;
        }
        for (int c = 0; c < cols_; ++c) {
            bool allZero = true;
            for (int r = 0; r < rows_; ++r) {
                if (at(r, c) != 0) { allZero = false; break; }
            }
            if (allZero) ++completed;
        }
        return completed;
    }

    int rows() const { return rows_; }
    int cols() const { return cols_; }
};

// ----------------- GAME FUNCTIONS -----------------
void loadGameHistory(const string& path = "score.txt") {
    ifstream in(path);
    if (!in) {
        cerr << RED << "Unable to open score file: " << path << "\n" << RESET;
        return;
    }
    string line;
    cout << CYAN << "----- High Scores -----\n" << RESET;
    while (getline(in, line)) {
        if (line.empty()) continue;
        string name;
        int score = 0;
        stringstream ss(line);
        vector<string> parts;
        string token;
        while (ss >> token) parts.push_back(token);
        if (!parts.empty()) {
            try { score = stoi(parts.back()); parts.pop_back(); }
            catch (...) { score = 0; }
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i) cout << " ";
                cout << MAGENTA << parts[i] << RESET;
            }
            cout << " won in " << GREEN << score << RESET << " lines.\n";
        }
    }
}

void playSinglePlayer() {
    string player;
    cout << CYAN << "Enter player name: " << RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, player);

    int rows = readInt("Enter number of rows: ");
    int cols = readInt("Enter number of columns: ");
    if (rows <= 0 || cols <= 0) { cout << RED << "Invalid board size.\n" << RESET; return; }

    int winLines = min(rows, cols);
    cout << GREEN << "First to complete " << winLines << " line(s) wins.\n" << RESET;
    cout << CYAN << "Press Enter to start..." << RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    BingoCard user(rows, cols), comp(rows, cols);
    user.generate(); comp.generate();

    int totalCells = rows * cols;
    int turn = 0;

    while (true) {
        if (turn == 0) {
            user.display(player + "'s card");
            int num = readInt(player + ", enter number to remove: ");
            if (num <= 0 || num > totalCells) { cout << RED << "Invalid number.\n" << RESET; continue; }
            if (!user.contains(num)) { cout << RED << "Number not found on your card.\n" << RESET; continue; }
            user.mark(num); comp.mark(num);
        }
        else {
            comp.display("Computer's card");
            vector<int> candidates;
            for (int r = 1; r <= totalCells; ++r)
                if (comp.contains(r)) candidates.push_back(r);
            if (!candidates.empty()) {
                int choose = candidates[rng() % candidates.size()];
                cout << MAGENTA << "Computer chooses: " << choose << RESET << "\n";
                comp.mark(choose); user.mark(choose);
            }
        }

        int userLines = user.countCompletedLines();
        int compLines = comp.countCompletedLines();
        if (userLines >= winLines && compLines >= winLines) {
            cout << YELLOW << "Draw!\n" << RESET; break;
        }
        else if (userLines >= winLines) {
            cout << GREEN << player << " won!\n" << RESET; break;
        }
        else if (compLines >= winLines) {
            cout << RED << "Computer won!\n" << RESET; break;
        }
        turn = 1 - turn;
    }
    cout << CYAN << "Returning to menu...\n" << RESET;
}

void playMultiplayer() {
    cout << CYAN << "Player 1 name: " << RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string p1; getline(cin, p1);
    cout << CYAN << "Player 2 name: " << RESET;
    string p2; getline(cin, p2);

    int rows = readInt("Enter number of rows: ");
    int cols = readInt("Enter number of columns: ");
    if (rows <= 0 || cols <= 0) { cout << RED << "Invalid board size.\n" << RESET; return; }
    int winLines = min(rows, cols);

    BingoCard c1(rows, cols), c2(rows, cols);
    c1.generate(); c2.generate();
    int total = rows * cols;
    int turn = (rng() % 2 == 0) ? 0 : 1;
    cout << MAGENTA << ((turn == 0) ? p1 : p2) << " wins the toss and starts.\n" << RESET;

    while (true) {
        if (turn == 0) {
            c1.display(p1 + "'s card");
            int num = readInt(p1 + ", enter number to remove: ");
            if (num <= 0 || num > total) { cout << RED << "Invalid number.\n" << RESET; continue; }
            if (!c1.contains(num)) { cout << RED << "Number not on your card.\n" << RESET; continue; }
            c1.mark(num); c2.mark(num);
        }
        else {
            c2.display(p2 + "'s card");
            int num = readInt(p2 + ", enter number to remove: ");
            if (num <= 0 || num > total) { cout << RED << "Invalid number.\n" << RESET; continue; }
            if (!c2.contains(num)) { cout << RED << "Number not on your card.\n" << RESET; continue; }
            c1.mark(num); c2.mark(num);
        }

        int l1 = c1.countCompletedLines(), l2 = c2.countCompletedLines();
        if (l1 >= winLines && l2 >= winLines) { cout << YELLOW << "Draw!\n" << RESET; break; }
        else if (l1 >= winLines) { cout << GREEN << p1 << " won!\n" << RESET; break; }
        else if (l2 >= winLines) { cout << GREEN << p2 << " won!\n" << RESET; break; }
        turn = 1 - turn;
    }
}

// ----------------- MAIN -----------------
int main() {
    while (true) {
        cout << BLUE << "**************** BINGO ****************\n" << RESET;
        cout << CYAN << "1) Start Game\n2) Load High Score\n3) How to play\n0) Exit\n" << RESET;
        int choice = readInt("Enter choice: ");
        if (choice == 0) break;
        switch (choice) {
        case 1: {
            cout << CYAN << "Enter 's' for single player or 'm' for multiplayer: " << RESET;
            char mode; cin >> mode;
            if (mode == 's') playSinglePlayer();
            else if (mode == 'm') playMultiplayer();
            else cout << RED << "Invalid mode.\n" << RESET;
            break;
        }
        case 2: loadGameHistory("score.txt"); break;
        case 3:
            cout << GREEN << "How to play:\n"
                << "- You and the opponent have a bingo card with unique numbers.\n"
                << "- Enter a number on your card to mark it; it will also be marked on opponent's card.\n"
                << "- Completing a row or column marks one line. First player to complete the required lines wins.\n" << RESET;
            break;
        default: cout << RED << "Unknown option.\n" << RESET;
        }
    }
    cout << BLUE << "Goodbye!\n" << RESET;
    return 0;
}
