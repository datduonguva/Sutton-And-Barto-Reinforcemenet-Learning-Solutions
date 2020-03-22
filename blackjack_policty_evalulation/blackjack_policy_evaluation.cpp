#include<cstdio>
#include<iostream>
#include<string>
#include<map>
#include<stdlib.h>
#include<time.h>
#include<vector>
#include<algorithm>
#include<numeric>
#include<fstream>
using namespace std;

class BlackJack{
public:
    BlackJack();
    ~BlackJack(){}
    void print_score_map();
    int draw_a_card();
    int dealer_turn(int state);
    int player_turn(int state, bool usable_one);
    int game_loop();
    void calculate_value_function();
private:
    double V[22][11][2];
    double gam = 1.0;
    vector<vector<vector<vector<double>>>> R;
};


void BlackJack::calculate_value_function(){
    for(int i = 0; i < 22; i++){
        for(int j = 0; j < 11; j++){
            for(int k = 0; k < 2 ; k++){
                V[i][j][k] = (R[i][j][k].size() == 0) ? 0: accumulate(R[i][j][k].begin(), R[i][j][k].end(), 0.0)/R[i][j][k].size();
            }
        }
    }

    string filename = "data.txt";
    ofstream outFile;
    outFile.open(filename.c_str());
    if (outFile.fail()){
        printf("Cannot open file!"); 
    }else{
        outFile << "[" ;
        for(int i = 0;i< 22; i++){
            outFile << "[" ;
            for(int j = 0;j < 11;j++){
                outFile << "[" ;
                for(int k = 0; k< 2; k++){
                    outFile << V[i][j][k] << ((k != 1)? "," : "]");
                }
                outFile << ((j != 10)? ",\n" : "]\n");
            }
            outFile << ((i != 21)? ",\n" : "]\n");
        }
    }
}

int BlackJack::game_loop(){
    // game simulation
    // first, the dealer got 2 cards
    vector<int> dealer_scores;

    int dealer_first_card = dealer_turn(0);
    dealer_scores.push_back(dealer_first_card);
    dealer_scores.push_back(dealer_first_card + dealer_turn(dealer_first_card));

    // next, set a score [12..21] for player, and whether he has a usable one
    vector<int> player_scores;
    vector<int> player_usable_one;
    player_scores.push_back((rand() % 10) + 12);
    player_usable_one.push_back(rand() % 2);

    // player takes first turn:
    int player_action = 0, dealer_action = 0;
    int counter = 0;
    int reward = 0;
    
    if (player_scores.back() == 21 && dealer_scores.back()== 21) reward = 0;
    else if (player_scores.back() == 21) reward = 1;
    else{
        while(1){
            player_action = player_turn(player_scores.back(), (player_usable_one.back() == 1));
            player_usable_one.push_back( (player_action == 1 || player_action == 11 || player_usable_one.back() == 1));
            player_scores.push_back(player_scores.back() + player_action);
            if (player_action == 0) break;
        }
        if (player_scores.back() > 21) reward = -1;
        else{
            // If he sticks not burst, dealer takes turns
            while(1){
                dealer_action = dealer_turn(dealer_scores.back());
                dealer_scores.push_back(dealer_scores.back() + dealer_action);
                if (dealer_action == 0) break;
            }
            if (dealer_scores.back() > 21 || dealer_scores.back() < player_scores.back()) reward = 1;
            else if (dealer_scores.back() > player_scores.back()) reward = -1;
        }

    }

    double g = 0;
    vector<int>::iterator it_score, it_usable_one;
    bool is_last = true; 
    
    for(int i = player_scores.size() - 1; i >= 0; i--){
        g = gam*g + ((is_last)? reward : 0);
        is_last = false;

        // For First-Visit Monte Carlos prediction, the state must not be seen at previous step
        // we only append the score to R when this states is new (hence the name "FIRST VISIT"
        it_score = find(player_scores.begin(), player_scores.begin() + i , player_scores[i]);
        it_usable_one = find(player_usable_one.begin(), player_usable_one.begin() + i , player_usable_one[i]);
        if ((it_score == (player_scores.begin() + i )) &&
            (it_usable_one == (player_usable_one.begin() + i )) &&
            (player_scores[i] <=21)){
            if (dealer_scores[0] == 11) dealer_scores[0] = 1;
            R[player_scores[i]][dealer_scores[0]][player_usable_one[i]].push_back(g);
        }
    }
    return reward;
}

BlackJack::BlackJack(){
    // Initialize vector of size [22, 11, 2, 0] to store the reward at each state.
    R = vector<vector<vector<vector<double>>>>(22, vector<vector<vector<double>>>(11, vector<vector<double>>(2, vector<double>(0))));
    printf("Shape of R: (%i, %i, %i)", R.size(), R[0].size(), R[0][0].size());
}

int BlackJack::player_turn(int state, bool usable_one){
    int action = 0;
    if (state < 20){
        action = draw_a_card();
        // 1 considered as 10 would burst if taking the action
        if (action == 1 && state + 11 <= 21) action = 11;
    }
    return action;
}

int BlackJack::dealer_turn(int state){
    int action = 0;
    if (state < 17){  // always hit a new card when the score < 17
        action = draw_a_card();
        if (action == 1 && state + 11 <= 21) action = 11;
    }    
    return action;
}

int BlackJack::draw_a_card(){
    return (rand() % 10 ) + 1;
}

int main(){
    srand(time(NULL));
    BlackJack a;
    vector<int> total_rewards;
    // Simulate 50000 games
    for(int i = 0; i<50000; i++){
        if (i%1000 == 0) cout << i<< endl;
        total_rewards.push_back(a.game_loop());
    }
    a.calculate_value_function();
}
