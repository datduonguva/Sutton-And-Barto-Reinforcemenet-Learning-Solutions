#include<iostream>
#include<vector>
#include<cstdlib>
#include<algorithm>
#include<string>
#include<fstream>
#include<cstdlib>
#include<bits/stdc++.h>
using namespace std;

typedef vector<int> v1_int;
typedef vector<v1_int> v2_int;
typedef vector<v2_int> v3_int;
typedef vector<v3_int> v4_int;

typedef vector<double> v1_double;
typedef vector<v1_double> v2_double;
typedef vector<v2_double> v3_double;
typedef vector<v3_double> v4_double;

class BlackJack_ES{
public:
    BlackJack_ES();
    int episode(int turn);
    int draw_a_card();
    void print_policy();
private:
    v3_int policy;
    v4_double Q;
    v4_int return_counter;
    double gamma = 1.0;
};

void BlackJack_ES::print_policy(){
    // This function print policy to file 
    string fn = "data.txt";
    ofstream outFile;
    outFile.open(fn.c_str());
    if (outFile.fail()){
        cout << "Cannot open file to write" << endl;
    }else{
        outFile << "[";
        for (int i = 0; i < 22; i++){
            outFile << "[" ;
            for(int j = 0; j < 11; j++){
                outFile << "[";
                for(int k = 0; k < 2; k++){
                    outFile << policy[i][j][k] ;
                    outFile << ((k == 1)? "]\n" : ",");
                }
                outFile << ((j== 10)? "]\n" : ", ");
            }
            outFile << ((i == 21)? "]\n": ", ") ;
        }
    }
    outFile.close();
}

BlackJack_ES::BlackJack_ES(){
    // create policy of size (22, 11, 2)
    policy = v3_int(22, v2_int(11, v1_int(2, 0)));

    // create Q value matrix of shape (22, 11, 2, 2)
    Q = v4_double(22, v3_double(11, v2_double(2, v1_double(2, 0))));

    // create the matrix (22, 11, 2, 2) storing the counter of (state, action)
    return_counter = v4_int(22, v3_int(11, v2_int(2, v1_int(2, 0))));
};

int BlackJack_ES::draw_a_card(){
    return  min(10, (rand() % 13) + 1);
}

int BlackJack_ES::episode(int turn){
    if (turn % 1000 == 0) cout << "turn: " << turn << endl;
    vector<int> dealer_scores;
    int dealer_ace = 0;
    vector<int> player_scores;
    vector<int> player_action; // 1 means hit, 0 mean stick
    vector<int> usable_one;
    int reward = 0;
    int action = 0;
    int dealer_first = 0;


    // prepare the initial state for the dealer
    dealer_scores.push_back(draw_a_card());
    dealer_scores.push_back(draw_a_card());

    if (dealer_scores[0] == 1 ||  dealer_scores[1] == 1){
        dealer_ace = 1;
    }

    dealer_scores[1] += dealer_scores[0];
    if (dealer_ace == 1) dealer_scores[1] += 10;
    dealer_first = dealer_scores[0];

    // prepare the initial state for the player
    player_scores.push_back(rand() % 10 + 12);
    usable_one.push_back((rand() %2));
    player_action.push_back((rand() % 2));
    
    // the game start with player's turn
    int usable_card, last_score, card_score;
    while (player_action.back() == 1){
        card_score = draw_a_card();
        usable_card = usable_one.back();
        last_score = player_scores.back();
        if (card_score == 1 && 11 + last_score <= 21){
            usable_card = 1;
            card_score = 11;
        }

        if ((card_score + last_score > 21) && (usable_card == 1)){
            last_score -= 10;
            usable_card = 0;
        }

        player_scores.push_back(last_score + card_score);
        usable_one.push_back(usable_card);

        if (player_scores.back() > 21) break;
        player_action.push_back(policy[player_scores.back()][dealer_first][usable_one.back()]);
    }
    
    // next, it is dealer's turn
    // dealer stop when its score is at least 17
    while (dealer_scores.back() < 17){
        card_score = draw_a_card();
        dealer_scores.push_back(dealer_scores.back() + card_score);
        if (dealer_ace == 0 && card_score == 1){
            dealer_scores[dealer_scores.size() -1] += 10;
            dealer_ace = 1;
        }
        
        if (dealer_ace == 1 && dealer_scores.back() > 21){
            dealer_scores[dealer_scores.size() -1] -= 10;
            dealer_ace = 0;
        }
    }

    // Get the reward of the game
    if (dealer_scores.back() == 21 && player_scores.back() == 21) reward = 0;
    else if (player_scores.back() == 21) reward = 1;
    else if (dealer_scores.back() == 21 || player_scores.back() > 21) reward = -1;
    else if (dealer_scores.back() > 21)  reward = 1;
    else if (dealer_scores.back() > player_scores.back() ) reward = -1;
    else if (dealer_scores.back() < player_scores.back()) reward = 1;
    else reward = 0;

    // learning phase
    double g = 0.0, old_q = 0.0;
    int counter = 0;

    for(int i = player_scores.size() - 1; i >= 0; i--){
        g = g*gamma + ((i == (player_scores.size() - 1))? 1.0*reward : 0.0);
        if (player_scores[i] <= 21){
            old_q = Q[player_scores[i]][dealer_first][usable_one[i]][player_action[i]];

            // increase the counter, then use it to calculate the q value
            counter = ++return_counter[player_scores[i]][dealer_first][usable_one[i]][player_action[i]];

            Q[player_scores[i]][dealer_first][usable_one[i]][player_action[i]] +=  ( g - old_q)/counter;

            // update the policy 
            vector<double> q_vec = Q[player_scores[i]][dealer_first][usable_one[i]];
            policy[player_scores[i]][dealer_first][usable_one[i]] = (q_vec[0] > q_vec[1])? 0: 1;
        }
    }   

    return reward;    
}


int main(int argc, char *argv[]){
    srand(time(NULL));

    BlackJack_ES bl;
    vector<int> game_reward;
    

    for(int i = 0; i < 1000000; i++){
        game_reward.push_back(bl.episode(i));
    }
    bl.print_policy();
}
