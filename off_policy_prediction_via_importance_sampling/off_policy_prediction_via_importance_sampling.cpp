#include<iostream>
#include<vector>
#include<string>
#include<algorithm>
#include<cstdlib>
#include<cmath>
#include<numeric>
#include<fstream>
using namespace std;

class SingleState{
public:
    double target_value();
    int target_episode();
    int random_policy(double &rho);
    double random_value_ordinary_importance_sampling();
    double random_value_weighted_importance_sampling();
private:
    int outcome(int pc);
};


double SingleState::random_value_weighted_importance_sampling(){
    double values[100][10000]; // storing the state values for 100 trials, each having 10000 episodes
    for(int trial = 0; trial < 100; trial ++){
        int n = 10000;
        double rhos[10000];
        double tmp_rho, denominator = 0.0;
        int tmp_reward = 0;
        for (int i = 0; i < n; i++){
            tmp_rho = 1.0;
            tmp_reward = random_policy(tmp_rho);
            values[trial][i] = values[trial][max(i-1, 0)] + tmp_rho*tmp_reward;

            rhos[i] = rhos[max(i-1, 0)] + tmp_rho;
        }

        for(int i = 0; i < n; i++){
            if (rhos[i] < 1e-6) values[trial][i] = 0;
            else values[trial][i] /= rhos[i];
        }
    }

    // write to file in json format
    string fn = "random_policy_weighted_importance_sampling.json";
    ofstream out_file;
    out_file.open(fn.c_str());
    if (out_file.fail()){
        cout << "Fail to open file" << endl;
    }else{
        out_file << "[";
        for(int i = 0; i < 100; i ++){
            out_file << "[";
            for(int j = 0; j < 10000; j++){
                out_file << values[i][j] << ((j == 9999) ? "]" : ",") ;
            }
            out_file << ((i == 99)? "] " : ",\n");
        }
    }

    out_file.close();
    return values[0][9999];

}

int SingleState::outcome(int pc){
    // Given the player scores, 
    // this function simulate the dealer's turn,
    // then find the reward of for the player
    int dc = 2, dace = 0;
    int card = 0, reward = 0;
    while (dc < 17){
        card = min(10, rand() % 13 + 1);
        dc += card;
        if (card == 1 && dace == 0){
            dc += 10;
            dace = 1;
        }

        if (dace == 1 && dc > 21){
            dc -= 10;
            dace = 0;
        }
    }
    
    if (pc == 21 && dc == 21) reward = 0;
    else if (pc == 21) reward = 1;
    else if (pc > 21) reward = -1;
    else if (dc > 21) reward = 1;
    else if (pc > dc) reward = 1;
    else if (dc > pc) reward = -1;

    return reward;
}   

double SingleState::random_value_ordinary_importance_sampling(){
    // This functions estimate the value fucntion for 10 time,
    // each with 10,000 episodes

    double values[100][10000];
    for(int trial = 0; trial < 100; trial ++){
        int n = 10000;
        double mean = 0;
        double rho;
        int reward = 0;
        for (int i = 1; i < n; i++){
            rho = 1.0;
            reward = random_policy(rho);
            // this equation utilizes the incremental way of calculating the mean
            // value of an array as specified in chapter 2
            values[trial][i] = values[trial][i-1] + (reward*rho - values[trial][i-1])/(i + 1);
        }
    }

    // write to file in json format

    string fn = "random_policy_ordinary_importance_sampling.json";
    ofstream out_file;
    out_file.open(fn.c_str());
    if (out_file.fail()){
        cout << "Fail to open file" << endl;
    }else{
        out_file << "[";
        for(int i = 0; i < 100; i ++){
            out_file << "[";
            for(int j = 0; j < 10000; j++){
                out_file << values[i][j] << ((j == 9999) ? "]" : ",") ;
            }
            out_file << ((i == 99)? "] " : ",\n");
        }
    }

    out_file.close();
    // return a values, just for demonstrating purpose
    return values[0][9999];
}

int SingleState::random_policy(double &rho){
    // This function generates a game using a behavior policy
    // where the action (hit or stick) is random
    int pc = 13, pace = 1;
    int card = 0, reward = 0, action = 0;
    rho = 1.0;
    while (true){
        action = random() % 2; // randomly choose an action (0 = stick, 1 = hit)

        // rho = (target_policy/ behavior policy)
        // for target policy, always hit if score < 20, else stick
        // for behavior policyy, hit or stick at equal probabity.
        
        if (pc >= 20){
            if (action == 0) rho /= 0.5;
            else rho *= 0;
        } else {
            if (action == 0) rho *= 0;
            else rho /=0.5;
        }

        if (action == 0) break;

        card = min(10, rand() %13 + 1);
        pc += card;

        if (pace == 0 && card == 1){
            pc += 10;
            pace = 1;
        }

        if (pace == 1 && pc > 21){
            pace = 0;
            pc -= 10;
        }

        if (pc > 21) break;
    }

    reward = outcome(pc);
    return reward;
}


double SingleState::target_value(){
    int n = 100000;
    double mean = 0.0;
    for (int i = 0; i < n; i++){
        // use the incremental way to calculate the mean value
        // of an array, to avoid storing the whole array in memory
        mean += (target_episode() - mean)/(i+1);
    }

    string fn = "target_value.json";
    ofstream out_file;
    out_file.open(fn.c_str());
    out_file << mean;
    out_file.close();
    return mean;

}

int SingleState::target_episode(){
    // Target policty: hit when player's card is less then 20, hit otherwise
    int pc = 13, pace = 1;
    int card = 0, reward = 0;
    while (pc < 20){
        card = min(10, rand() % 13 + 1);
        pc += card;

        if (pace == 0 && card ==1){
            pc += 10;
            pace = 1;
        }
        if (pace == 1 && pc > 21){
            pace = 0;
            pc -= 10;
        }
    }

    reward = outcome(pc);

    return reward;
}

int main(){
    srand(time(NULL));
    SingleState single_state;
    cout << single_state.random_value_ordinary_importance_sampling() << endl;
    cout << single_state.random_value_weighted_importance_sampling() << endl;
    cout << single_state.target_value() << endl;

}
