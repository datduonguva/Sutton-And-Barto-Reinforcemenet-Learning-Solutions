#include<iostream>
#include<vector>
#include<string>
#include<chrono>
#include<cmath>
#include<algorithm>
#include<random>
#include<fstream>

using namespace std;

class Grid{
public:
    Grid();
    void episode(int);
    vector<int> episode_counts;
    void save_to_json();
private:
    double epsilon = 0.1;
    double alpha = 0.5;
    double scale = -10.0; // for initializing Q[S,A];

    int moves[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    int wind[10] = {0, 0, 0, -1, -1, -1, -2, -2, -1, 0};
    double Q[7][10][4]; // (height, width, action)
    default_random_engine generator;
    uniform_real_distribution<double> distribution;
    double get_rand(double);
    int soft_policy(int *states);

};

// Helper function to save the episode vs steps to json
// for later ploting
void Grid::save_to_json(){
    ofstream out_file;
    string fn = "episode_vs_steps.json";
    out_file.open(fn.c_str());
    out_file << "[";
    for(int i = 0; i < episode_counts.size()-1; i++){
        out_file << episode_counts[i] << ", " << endl;
    }

    out_file << episode_counts.back() << "]" << endl;
}

// Run one episode from the starting point
// to the goal
void Grid::episode(int episode_count){

    // the starting point is at [3, 0]
    int states[] = {3, 0};
    int new_states[2];

    // Get an action
    int action = soft_policy(states);
    int new_action;
    double current_q = 0.0;
    while (true){

        // Keep track of the episode
        episode_counts.push_back(episode_count);

        // The new states must be within the grid, [0 -> 6] for vertical direction
        // and [0 -> 10] for for the horizontal direction

        new_states[0] = min(6, max(0, states[0] + moves[action][0]+ wind[states[1]]));
        new_states[1] = min(9, max(0, states[1] + moves[action][1]));

        // getting a new action after moving to the new state
        new_action = soft_policy(new_states);
       
        // update the Q table
        Q[states[0]][states[1]][action] += alpha*(-1.0 + Q[new_states[0]][new_states[1]][new_action] - Q[states[0]][states[1]][action]);

        // replace the states
        states[0] = new_states[0];
        states[1] = new_states[1];
        action = new_action;

        // Check if the goal is reached
        if (states[0] == 3 && states[1] == 7){
            break;
        }
    }

}

// epsilon-greedy policy
int Grid::soft_policy(int *states){

    // For a small probability, choose randome action
    if (get_rand(1.0) < epsilon){
        return (int) get_rand(4.0);
    }else{

        // greedy policy: choose action whose action values is maximum amongs
        // current state.
        double max_q = Q[states[0]][states[1]][0];
        int max_state = 0;
        for(int i = 1; i < 4; i++){
            if (Q[states[0]][states[1]][i] > max_q){
                max_q = Q[states[0]][states[1]][i];
                max_state = i; 
            }
        }
        return max_state;
    }
}

// helper to generate random number from [0.0, scale)
double Grid::get_rand(double scale){
    return distribution(generator)*scale;
}

Grid::Grid(){
    // initialize Q
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    generator = default_random_engine(seed);
    distribution = uniform_real_distribution<double>(0.0, 1.0);
    for (int i = 0; i < 7; i++){
        for(int j = 0; j< 10; j++){
            for(int k = 0; k < 4; k++){
                Q[i][j][k] = get_rand(-0.0);
            }
        }
    }

    // Q terminal is 0
    for(int i = 0; i < 4; i++) Q[3][7][i] = 0;
}

int main(){
    Grid grid;
    for (int i = 0 ; i < 1000; i++){
        grid.episode(i);
    }
    grid.save_to_json();
    return 0;
}
