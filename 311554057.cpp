#include <iostream>
#include <vector>
#include <ctime>
#include <map>
#include <string>
#include <chrono>

using namespace std;

const int SIZE = 3;

struct action {
    vector<vector<int>> board ;
    int row_col ;
    int subtract ;
};

void random_init(vector<vector<int>>& board) {
    	
	srand((unsigned) time(NULL));


    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = 50 + (rand() % 50);
        }
    }
}

void given_init(vector<vector<int>>& board){
    vector<vector<int>> given{
        {2, 4, 3}, 
        {4, 2, 4}, 
        {3, 4, 2}
    };

    board = given;
}

void print_board(vector<vector<int>>& board) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            cout << board[i][j] << " ";
        }
        cout << endl;
    }
}

bool check_valid(vector<vector<int>> board, int row_or_col, int subtract){
    int min = INT_MAX;
    if (subtract <= 0 || subtract > 3){  // subtract not 1, 2 or 3
        return false;
    }
    if (row_or_col >= 0 && row_or_col < SIZE){  // row
        for (int i = 0; i < SIZE; i++) {
            min = board[row_or_col][i]<min ? board[row_or_col][i] : min;
        }

        if (min == 0){
            return false;
        }
        else if (min == 1 && subtract > 1){
            return false;
        }
        else if (min == 2 && subtract > 2){
            return false;
        }
        return true;
    }
    else if (row_or_col >= SIZE && row_or_col < SIZE*2){  // col
        row_or_col -= SIZE;
        for (int i = 0; i < SIZE; i++) {
            min = board[i][row_or_col]<min ? board[i][row_or_col] : min;
        }

        if (min == 0){
            return false;
        }
        else if (min == 1 && subtract > 1){
            return false;
        }
        else if (min == 2 && subtract > 2){
            return false;
        }
        return true;
    }
    else{
        return false;
    }

}

void board_subtract(vector<vector<int>>& board, int row_or_col, int subtract){
    if (row_or_col >= 0 && row_or_col < SIZE){  // row
        for (int i = 0; i < SIZE; i++) {
            board[row_or_col][i] -= subtract;
        }
    }
    else if (row_or_col >= SIZE && row_or_col < SIZE*2){  // col
        row_or_col -= SIZE;
        for (int i = 0; i < SIZE; i++) {
            board[i][row_or_col] -= subtract;
        }
    } 
}

bool check_diagonal(vector<vector<int>>& board) {
    bool diagonal1 = true;
    bool diagonal2 = true;
    for (int i = 0; i < SIZE; i++) {
        diagonal1 &= (board[i][i] == 0);
        diagonal2 &= (board[i][SIZE - i - 1] == 0);
    }
    return diagonal1 || diagonal2;
}

bool check_row(vector<vector<int>>& board, int row) {  // check num in row are all 0
    for (int i = 0; i < SIZE; i++) {
        if (board[row][i] != 0) {
            return false;
        }
    }
    return true;
}

bool check_col(vector<vector<int>>& board, int col) {
    for (int i = 0; i < SIZE; i++) {
        if (board[i][col] != 0) {
            return false;
        }
    }
    return true;
}

bool check_game_end(vector<vector<int>>& board, bool &dead_end) {
    bool game_end = false;
    bool row_end = true;
    bool col_end = true;

    // end condition 1 : all the numbers in any row, column, or diagonal become 0
    for (int i = 0; i < SIZE; i++) {
        game_end |= check_row(board, i);
        game_end |= check_col(board, i);
    }
    game_end |= check_diagonal(board);

    // end condition 2 : every row or column contains the number 0
    if(!game_end){
        for (int i = 0; i < SIZE; i++) {
            bool row_exist_zero = false;
            bool col_exist_zero = false;
        
            for (int j = 0; j < SIZE; j++){
                if(board[i][j] == 0) row_exist_zero = true;
                if(board[j][i] == 0) col_exist_zero = true;
            }

            if (row_exist_zero == false) row_end = false;
            if (col_exist_zero == false) col_end = false;

        }
        dead_end = row_end && col_end;
        game_end |= dead_end;
    }


    return game_end;
}

vector<action> generate_states(vector<vector<int>> board) {
    vector<action> vaild_state ;
    vector<vector<int>> temp_board ;
    action temp_state ;
    for ( int i = 0 ; i < SIZE*2 ; i++ ) {
        bool has_zero = (i <= 2) ? check_row(board, i) : check_col(board, i-3) ;
        if (!has_zero) {
            for ( int subtract = 1 ; subtract <= 3 ; subtract++ ) {
                if (check_valid(board, i, subtract)) {
                    vector<vector<int>> temp_board = board ;
                    board_subtract(temp_board, i, subtract) ;
                    
                    temp_state.board = temp_board ;
                    temp_state.row_col = i ;
                    temp_state.subtract = subtract ;
                    
                    vaild_state.push_back(temp_state) ;
                }
            }
        }
    }
    
    return vaild_state ;
}

class TreeNode {
public:
    vector<vector<int>> board ;
    TreeNode* parent ;
    map<pair<int, int>, TreeNode*> children ;
    bool is_terminal ;
    bool is_fully_expanded ;
    bool dead_end = false ;
    int visits ;
    double score ;
    
    // init TreeNode
    TreeNode(vector<vector<int>> initial_state, TreeNode* parent_node) {
        board = initial_state ;
        is_terminal = check_game_end(board, dead_end) ;
        is_fully_expanded = is_terminal ;
        parent = parent_node ;
        visits = 0 ;
        score = 0 ;
        children = {} ;
    }
};

class MCTS {
private:
    TreeNode* root ;
    
    TreeNode* select(TreeNode* node) {
        while (!node->is_terminal) {
            if (node->is_fully_expanded) {
                node = get_best_move(node, 2) ;
            }
            else {
                return expand(node) ;
            }
        }
        
        return node ;
    }
    
    TreeNode* expand(TreeNode* node) {
        // 生成所有合法狀態
        vector<action> vaild_state = generate_states(node->board) ;
        for ( auto& state : vaild_state ) {
            pair<int, int> key = make_pair(state.row_col, state.subtract) ;
            if (node->children.find(key) == node->children.end()) {
                TreeNode* new_node = new TreeNode(state.board, node) ;
                node->children[key] = new_node ;
                if (vaild_state.size() == node->children.size()) {
                    node->is_fully_expanded = true ;
                }
                
                return new_node ;
            }
        }
        
        return nullptr ; // should not reach here
    }
    
    double rollout(TreeNode* node) {
        vector<vector<int>> board = node->board ;
        int player = 0 ; // AI go first
        int total_cost[2] = {0} ;
        bool game_end = false ;
        bool dead_end = false ;
        int reward = 15 ;
        int penalty = 7 ;
        double result = 0.0 ;
        while(!game_end) {
            vector<action> vaild_state = generate_states(board) ;
            if (vaild_state.empty()) {
                return result ;
            }
            
            action random_action = vaild_state[rand() % vaild_state.size()] ;
            board_subtract(board, random_action.row_col, random_action.subtract) ;
            total_cost[player] += random_action.subtract ;
            
            game_end = check_game_end(board, dead_end) ;
            
            if (!game_end) {
                player = (player + 1) % 2 ;
            }
        }
        
        if (!dead_end) {
            total_cost[player] -= reward ;
        }
        else {
            total_cost[player] += penalty ;
        }
        
        // 比較cost決定誰贏
        if (total_cost[0] < total_cost[1]) {
            result = 1 ;
        }
        else if (total_cost[0] > total_cost[1]) {
            result = -1 ;
        }
        
        return result ;
    }
    
    void backpropagate(TreeNode* node, double score) {
        while (node != nullptr) {
            node->visits += 1 ;
            node->score += score ;
            node = node->parent ;
        }
    }
    
    TreeNode* get_best_move(TreeNode* node, double c) {
        TreeNode* best_move = nullptr ;
        double best_score = -INFINITY ;
        for (auto& child_node : node->children) {
            double move_score = child_node.second->score / child_node.second->visits + c * sqrt(log(node->visits) / child_node.second->visits) ;
            
            if (move_score > best_score) {
                best_score = move_score ;
                best_move = child_node.second ;
            }
            
            if (move_score == 0 || move_score == 1) {
                return child_node.second ;
            }
        }
        
        return best_move ;
    }
    
public:
    MCTS() : root(nullptr) {}
    
    void search(vector<vector<int>> initial_state) {
        chrono::duration<double> time_limit = chrono::seconds(55) ;
        auto start_time = chrono::steady_clock::now() ;
        
        root = new TreeNode(initial_state, nullptr) ;
        while(true) {
            TreeNode* node = select(root) ;
            double score = rollout(node) ;
            backpropagate(node, score) ;
            
            auto current_time = chrono::steady_clock::now() ;
            chrono::duration<double> elapsed_time = current_time - start_time ;
            if (elapsed_time >= time_limit)
                break;
        }
    }
    
    pair<int, int> get_best_action() {
        TreeNode* best_child = get_best_move(root, 0) ;
        for (auto& child_node : root->children) {
            if (child_node.second == best_child) {
                return child_node.first ;
            }
        }
        
        return make_pair(-1, -1) ; // should not reach here
    }
};

void make_your_move(vector<vector<int>> board, int &row_or_col, int &subtract){
    MCTS mcts ;
    mcts.search(board) ;
    pair<int, int> action = mcts.get_best_action() ;
    row_or_col = action.first ;
    subtract = action.second ;
}

void opponent_move(vector<vector<int>> board, int &row_or_col, int &subtract){

    cin >> row_or_col;
    cin.get();

    cin >> subtract;
    cin.get();

}

int main() {
    vector<vector<int>> board(SIZE, vector<int>(SIZE));
    int player = 0; // player 0 goes first
    int total_cost[2] = {0}; // total cost for each player
    bool your_turn = true;
    bool game_end = false;
    bool dead_end = false;
    int reward = 15;
    int penalty = 7;

    cout << "Board initialization..." << endl << endl;

    random_init(board);  // initialize board with positive integers
    //given_init(board);   // initialize board with given intergers (for testing only)

    while (!game_end) {
        cout << "Current board:" << endl;
        print_board(board);
        cout << endl << "Player " << player << "'s turn:" << endl;
        system("pause");

        int row_or_col;  // row1, row2, row3 -> 0, 1, 2 ; col1, col2, col3 -> 3, 4, 5
        int subtract;  // number to subtract
        bool valid;  // legal move checking

        if(your_turn){
            time_t start, end, diff;
            start = time(nullptr);
            make_your_move(board, row_or_col, subtract);
            end = time(nullptr);

            cout << "Time: " << difftime(end, start) << " seconds" << endl << endl;            
        }
        else{
            //time_t start, end, diff;
            //start = time(nullptr);
            opponent_move(board, row_or_col, subtract);
            //end = time(nullptr);

            //cout << "Time: " << difftime(end, start) << " seconds" << endl << endl; 
        }


        cout << "Player " << player << "'s move: ";
        cout << "row_or_col: " << row_or_col << " subtract: " << subtract << endl << endl;

        cout << "Valid checking..." ;
        valid = check_valid(board, row_or_col, subtract);
        if(valid) cout << "The move is vaild." << endl;
        else cout << "The move is invalid, game over." << endl;
        system("pause");

        board_subtract(board, row_or_col, subtract);

        // update player's total cost
        total_cost[player] += subtract;
        cout << "Player 0 total cost: " << total_cost[0] << endl;
        cout << "Player 1 total cost: " << total_cost[1] << endl;
        cout << "--------------------------------------" << endl;

        // check if game has ended
        game_end = check_game_end(board, dead_end);

        if(!game_end){
            // switch to other player
            your_turn = !your_turn;
            player = (player + 1) % 2;
        }
        
        
    }

    // print final board and result
    cout << "Final board:" << endl;
    print_board(board);
    if (!dead_end) {
        cout << "Player " << player << " ends with a diagonal/row/col of 0's!" << endl;
        total_cost[player] -= reward;
    }
    else {
        cout << "Player " << player << " ends with a dead end!" << endl;
        total_cost[player] += penalty;
    }
    
    cout << "Player 0 total cost: " << total_cost[0] << endl;
    cout << "Player 1 total cost: " << total_cost[1] << endl;
    system("pause");

    return 0;
}
