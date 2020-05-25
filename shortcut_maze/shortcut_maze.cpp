#include<iostream>
#include<set>
#include<numeric>
#include<vector>
#include<string>
#include<chrono>
#include<cmath>
#include<algorithm>
#include<random>
#include<fstream>

using namespace std;
typedef struct state_t {int x; int y;} state_t;
typedef struct model_element{
    double reward;
    state_t state;
} model_element;

enum action_t {UP, DOWN, LEFT, RIGHT};

class Environment{
public:
    Environment();
    void perform_action(state_t,action_t, state_t&, double&);
    void set_is_blocked(int, int, int);
    int get_blocked(int x, int y){return is_blocked[y][x];}
private:
    static const int width=9;
    static const int height=6;
    state_t goal {8, 0};
    int is_blocked[height][width]; 
};

class Agent{
public:
    Agent(bool);
    void time_step(int step, Environment env);
    void save_to_json(int run);
private:
    default_random_engine generator;
    uniform_real_distribution<double> distribution;
    double epsilon = 0.1;
    double alpha_= 1.0;
    double gamma_= 0.95;
    double kappa_ = 1e-3; 
    double get_rand(double);
    static const int width = 9;
    static const int height = 6;
    static const int actions = 4; //up, down, left, right
    bool is_enhanced=false;
    int planning_steps = 30;
    state_t current_state {3, 5};
    state_t new_state;
    action_t current_action;
    double reward;
    vector<double> cumulative_reward;
    double Q[height][width][actions];
    bool is_observed[height][width];
    int last_seen[height][width][actions];
    vector<action_t> previous_actions[height][width];
    action_t get_action(state_t);
    vector<state_t> history;
    void record_action(state_t, action_t);
    model_element model[height][width][actions]; 
    state_t observed_state_sampling();
    action_t get_observed_action(state_t);
};

void Environment::set_is_blocked(int x, int y, int value){
    is_blocked[y][x] = value;
}
action_t Agent::get_observed_action(state_t state){
    int random_index = (int) get_rand(previous_actions[state.y][state.x].size()*1.0);
    return previous_actions[state.y][state.x][random_index];
}

void Agent::record_action(state_t state, action_t action){
    if (previous_actions[state.y][state.x].size() == 4) return;
    bool is_seen = false;
    for(int i = 0; i < previous_actions[state.y][state.x].size(); i++){
        if (previous_actions[state.y][state.x][i] == action) is_seen = true;
    }

    if (!is_seen) previous_actions[state.y][state.x].push_back(action);
}

state_t Agent::observed_state_sampling(){
    vector<int> visited;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if (is_observed[i][j]) visited.push_back(i*width + j);
        }
    }
    int random_index = visited[(int) (get_rand(visited.size()*1.0))];
    state_t result{random_index % width, random_index/ width};
    return result;
}

void Agent::save_to_json(int run){
    ofstream out_file;
    string fn = "reward_vs_step_0" + to_string(run) + ".json";
    if (is_enhanced) fn = "reward_vs_step_1" + to_string(run) + ".json";
    out_file.open(fn.c_str());
    out_file << "[";
    for(int i = 0; i < cumulative_reward.size()-1; i++){
        out_file << cumulative_reward[i] << ", " << endl;
    }   
    out_file << cumulative_reward.back() << "]" << endl;

}


void Agent::time_step(int step, Environment env){
    // a current_state must be marked as observed
    is_observed[current_state.y][current_state.x] = true;

    // store the his tory of the model, for visualizing purpose
    state_t tmp_state {current_state.x, current_state.y};
    history.push_back(tmp_state);
    
    // Get an action following epsilon-greedy strategy
    current_action = get_action(current_state);

    // Perfon the action, observe the new_state and reward
    env.perform_action(current_state, current_action, new_state, reward);

    // Record the action performed in this current_state, will be used for planning
    record_action(current_state, current_action);

    // Record the time step where the action was performed on this current_state
    last_seen[current_state.y][current_state.x][current_action] = step;

    // Update the Q table
    double max_q = *max_element(Q[new_state.y][new_state.x], Q[new_state.y][new_state.x] + actions);
    double current_q = Q[current_state.y][current_state.x][current_action];
    Q[current_state.y][current_state.x][current_action] += alpha_*(reward + gamma_*max_q  - current_q);

    // Record the cumulative reward, for visualizing purpose only
    if (cumulative_reward.size() == 0) cumulative_reward.push_back(reward);
    else cumulative_reward.push_back(cumulative_reward.back() + reward);

    // Model learning step
    model[current_state.y][current_state.x][current_action].reward = reward;
    model[current_state.y][current_state.x][current_action].state.x = new_state.x;
    model[current_state.y][current_state.x][current_action].state.y = new_state.y;
    
    // planning steps
    for (int i_n = 0; i_n < planning_steps; i_n++){
        // select a random state
        state_t random_state = observed_state_sampling();

        // select a random action performed in that selected state
        action_t random_action = get_observed_action(random_state);
        model_element model_tmp = model[random_state.y][random_state.x][random_action];

        // for the enhanced model, increase reward by an amount proportional to the sqrt(elapsed time step) 
        double reward_tmp = model_tmp.reward;
        if (is_enhanced) reward_tmp += kappa_*sqrt(1.0*step - 1.0*last_seen[random_state.y][random_state.x][random_action]);

        // update the Q table 
        max_q = *max_element(Q[model_tmp.state.y][model_tmp.state.x], 
                             Q[model_tmp.state.y][model_tmp.state.x] + actions);

        Q[random_state.y][random_state.x][random_action] += \
            alpha_*(reward_tmp + gamma_*max_q - Q[random_state.y][random_state.x][random_action]);
    }

    // move to the new state
    current_state.x = new_state.x;
    current_state.y = new_state.y;
}

void Environment::perform_action(state_t current_state, action_t action, state_t& new_state, double& reward){
    int dx = 0, dy = 0;
    if (action == UP) dy = -1;
    else if (action == DOWN) dy = 1;
    else if (action == LEFT) dx = -1;
    else if (action == RIGHT) dx = 1;

    new_state.x = min(8, max(0, current_state.x + dx));
    new_state.y = min(5, max(0, current_state.y + dy));

    // If the new position is blocked, move back to the previous step
    if (is_blocked[new_state.y][new_state.x] == 1){
        new_state.x = current_state.x;
        new_state.y = current_state.y;
    }

    // All steps have reward 0.0 except for the goal whose reward is 1.0
    reward = 0.0;
    if (new_state.x == goal.x && new_state.y == goal.y){
        reward = 1.0;

        // if falls to the goal, move to the starting point
        new_state.x = 3;
        new_state.y = 5;
    }
}

// Get action based on epsilon-greedy strategy
action_t Agent::get_action(state_t state){
    if (get_rand(1.0) < epsilon){
        return static_cast<action_t> ((int) (get_rand(actions*1.0)));
    }else{
        int selected_action = (max_element(Q[state.y][state.x], Q[state.y][state.x] + 4) - Q[state.y][state.x]);
        return static_cast<action_t>(selected_action);
    }
}

Agent::Agent(bool is_enhanced){
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    generator = default_random_engine(seed);
    distribution = uniform_real_distribution<double>(0.0, 1.0);
    // Initialize the Q table and is_observed to false
    for (int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            for(int k = 0; k < actions; k++){
                // The choice of the random initilization here is pretty important
                // We want the model to learn but also want it to explore in the beginning
                // By giving it large randomization like this,
                // The model won't quickly converge and spend more time
                // exploring the environment.
                Q[i][j][k] = get_rand(5.0);
            }

            is_observed[i][j] = false;
        }
    }
    this->is_enhanced = is_enhanced;
}

double Agent::get_rand(double scale){
    return distribution(generator)*scale;
}

Environment::Environment(){
    //initially, all there is block from [3, 1] -> [3, 9]
    for (int i = 1; i <width; i++){
        is_blocked[3][i] = 1;
    }
}

int main(){
    // We will run 10 times,
    // export the expected reward to json 
    // then use python to plot the average
    for(int run = 0; run < 10; run++){
        cout << "run: " << run << endl;
        Environment env;
        Agent agent(false);
        for(int step=0; step < 6000; step ++){
            agent.time_step(step, env);

            // at step 3000, open the shortcut
            if (step == 3000){
                env.set_is_blocked(8, 3, 0);
            }
        }
        agent.save_to_json(run);
        Environment env1;
        Agent agent1(true);
        for(int step=0; step < 6000; step ++){
            agent1.time_step(step, env1);

            // at step 3000, open the shortcut
            if (step == 3000){
                env1.set_is_blocked(8, 3, 0);
            }
        }
        agent1.save_to_json(run);
    }
}
