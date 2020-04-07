// The map for the race is a Json array of a NxM array
// storing the value of 0 or 1. 
// A cell that the car can move to is denoted by 1, 0 otherwise
// the car starts on any cells on the lowest rows, 
// and finish at any cell on the right-most columns.
// Note: because the origin of the coordinate is at the top left
// of the map, +x pointing right and +y pointing down,
// the condition of non negative v_y become non positive v_y

#include<iostream>
#include<string>
#include<algorithm>
#include<numeric>
#include<cstdlib>
#include<cmath>
#include<fstream>
#include<vector>
using namespace std;

typedef vector<double> double1;
typedef vector<double1> double2;
typedef vector<double2> double3;
typedef vector<double3> double4;
typedef vector<double4> double5;
typedef vector<int> int1;
typedef vector<int1> int2;
typedef vector<int2> int3;
typedef vector<int3> int4;
typedef vector<int4> int5;

class RaceCar{
public:
    RaceCar(string filename);
    int soft_policy(int * pos, int * vel, double &);
    int episode();
    void get_random_start_position(int *);
    void get_optimal_policy_track(string );
private:
    int2 track;
    int n_rows, n_cols;
    int n_action = 9;
    int max_speed = 5;
    int action_set[9][2] = {{-1, -1},{-1,0}, {-1, 1}, {0, -1}, {0, 0}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    double5 Q;
    double5 C;
    int4 policy;
    double gamma_ = 0.9;
    int starting_x[2]; // starting_x is the horizontal components of the starting line ends
    int ending_y[2]; // ending_y is the vertical components of the ending_y
};

void RaceCar::get_optimal_policy_track(string fn){
    // This function prints path to the json file
    ofstream out_file;
    out_file.open(fn.c_str());
    out_file << "{";
    bool begin_index = true;
    for(int postion_x = starting_x[0]; postion_x <= starting_x[1]; postion_x ++ ){
        if (begin_index){
            out_file << "\"" << postion_x << "\":[";
            begin_index = false;
        } else out_file << ",\"" << postion_x << "\":[";

        int positions[2] = {n_rows -1, postion_x};
        int speed[2] = {0, 0};
        vector<vector<int>> speed_history;
        vector<vector<int>> position_history;
        vector<int> action_history;

        
        // Generate an episode
        int counter = 0;
        while (counter < 1000){
            speed_history.push_back(vector<int>(speed, speed + 2));
            position_history.push_back(vector<int>(positions, positions + 2));

            int action_index = policy[positions[0]][positions[1]][-speed[0]][speed[1]];
            action_history.push_back(action_index);
            
            speed[0] += action_set[action_index][0];
            speed[1] += action_set[action_index][1];

            if( speed[0]  > 0 || speed[1] < 0 ||
                (speed[0] == 0 && speed[1] == 0)){
                cout << "invalid speed: " << speed[0] << ", " << speed[1] << endl;
                break;
            }
            
            positions[0] += speed[0];
            positions[1] += speed[1];
            

            if ( positions[0] < 0 || 
                 positions[0] >= n_rows || 
                 positions[1] < 0 ||
                 (positions[1] >= n_cols && (positions[0]< ending_y[0] || positions[0] > ending_y[1])) ||
                 (track[positions[0]][positions[1]] == 0)
            ){
                break;
            } else if (positions[1] >= n_cols - 1  && positions[0] >= ending_y[0] &&  positions[0] <= ending_y[1]){
                positions[1] = n_cols-1;
                position_history.push_back(vector<int>(positions, positions + 2));
                break;
            }
            
            counter += 1;
        }

        // Print the episode to file
        for (int i = 0; i < position_history.size(); i++){
            out_file << "[" << position_history[i][0] << "," << position_history[i][1] << "]";
            if (i != position_history.size() -1) out_file << ",";
            else out_file << "]";
        }
    }

    out_file << "}";
    out_file.close();
}

void  RaceCar::get_random_start_position(int * arr){
    // this function random moves the car to the starting point
    // where y = n_rows -1 , and x is randomly between starting_x[0] to starting_x[1]
    arr[0] = n_rows - 1;
    arr[1] = (rand() % (starting_x[1] - starting_x[0] + 1) + starting_x[0]);
}
int RaceCar::episode(){
    int speed[] = {0, 0};
    int positions[2];
    double probability;
    get_random_start_position(positions);
    vector<vector<int>> speed_history;
    vector<vector<int>> position_history;
    vector<int> action_history;
    vector<double> prob_history;
    
    // Generate an episode
    int counter = 0;

    while (1){
        speed_history.push_back(vector<int>(speed, speed + 2));
        position_history.push_back(vector<int>(positions, positions + 2));

        int action_index = soft_policy(positions, speed, probability );
        action_history.push_back(action_index);
        prob_history.push_back(probability);
        
        speed[0] += action_set[action_index][0];
        speed[1] += action_set[action_index][1];

        positions[0] += speed[0];
        positions[1] += speed[1];
        
        // if hit barier
        if ( positions[0] < 0 || 
             positions[0] >= n_rows || 
             positions[1] < 0 ||
             (positions[1] >= n_cols && (positions[0]< ending_y[0] || positions[0] > ending_y[1])) ||
             (track[positions[0]][positions[1]] == 0)
        ){
            // move to the starting point
            get_random_start_position(positions);
            speed[0] = 0;
            speed[1] = 0;
        } else if (positions[1] >= n_cols - 1  && positions[0] >= ending_y[0] &&  positions[0] <= ending_y[1]){
            // if hit the finish line, break
            break;
        }
        
        counter += 1;
    }
    
    // Update the target policy
    double G = 0.0;
    double W = 1.0;
    int p_y, p_x, v_y, v_x, a_t, optimal_a;;

    for(int t = position_history.size() -1 ; t >= 0; t--){
        G = gamma_*G + (-1.0);
        p_y = position_history[t][0];
        p_x = position_history[t][1];
        v_y = -speed_history[t][0];
        v_x = speed_history[t][1];
        a_t = action_history[t];
        optimal_a = 0;
        C[p_y][p_x][v_y][v_x][a_t] += W;
        Q[p_y][p_x][v_y][v_x][a_t] += W/C[p_y][p_x][v_y][v_x][a_t]*(G - Q[p_y][p_x][v_y][v_x][a_t]);
        for(int a_i = 0; a_i < 9; a_i++){
            if (Q[p_y][p_x][v_y][v_x][a_i] > Q[p_y][p_x][v_y][v_x][optimal_a]) optimal_a = a_i;
        }
        policy[p_y][p_x][v_y][v_x] = optimal_a;
        if (a_t != optimal_a) break;
        W *= 1/prob_history[t];
    }
    return counter;
}


int RaceCar::soft_policy(int pos[], int vel[], double & probability){
    // The soft policy is:
    // for 10% of the time, return acceleration of [0, 0]
    // for 90% of the time, 
        // for 90% of the time, follow the current Q value
        // for 10% of the time, randomlly pick an acceleration in action_set
    // Check to see if the selected action is value (both component of velocity is non negative, less than 5)
    // If not valid, retry. 
    while (1){
        int y_axis = rand() % 10;
        int x_axis = rand() % 10;

        int y_acc = 0, x_acc = 0;
        probability = 0.1;
        
        int a_star = 4;

        if (rand() % 10 != 0){
            a_star = policy[pos[0]][pos[1]][-vel[0]][vel[1]];
            probability = 0.9*0.9;
            // 20% of the time, generate some other state
            if (rand() % 10 < 1 ){
                int tmp_a_start = rand() % 9;
                if (tmp_a_start == a_star) probability = 0.9*(0.9 + 0.1/9);
                else probability = 0.9*0.1/9;

                a_star = tmp_a_start;
            }

            y_acc = action_set[a_star][0];
            x_acc = action_set[a_star][1];
        }

        // Check to see the conditions mentioned above are satisfied.
        // Note: because of the way I put the origin to be on the Top-left, of the image
        // +x pointing right, + y pointing down, the velocity will be negative
        // if the car move up.
        // That is why the condition of nonegative v_y is tranlated to non-positive
        // velocity in y dimension
        if (!((x_acc + vel[1] == 0) && (y_acc + vel[0] == 0)) &&
            (x_acc + vel[1] < max_speed)                      &&
            (x_acc + vel[1] >= 0)                             &&
            (y_acc + vel[0] <= 0)                             &&
            (y_acc + vel[0] > -max_speed)
           ){
            return a_star;
        }
    }
}

RaceCar::RaceCar(string filename){
    // Read the map, set the n_rows, n_cols, the track
    ifstream input_file;
    input_file.open(filename.c_str());
    string line;
    getline(input_file, line);

    vector<int> tmp;
    n_rows = 0;
    n_cols = 0;

    // This parses out the map that is in nested json list format
    for (int i = 0; i < line.size(); i++){
        if (line[i] == '0' || line[i] == '1') tmp.push_back(stoi(line.substr(i, i+1)));
        if (line.substr(i, 2) == "],") {
            n_rows += 1;
        }
    }
    n_rows += 1;
    n_cols = tmp.size()/n_rows;

    // Read the track
    for(int row = 0; row < n_rows; row++){
        vector<int> tmp_row;
        for(int col = 0; col < n_cols; col++){
            tmp_row.push_back(tmp[row*n_cols + col]);
        }
        track.push_back(tmp_row);
        // The starting points is on the last rows. Read the starting_x location
        // , store the first and last point from left to right to starting_x[0] and starting_x[1]
        if (row == n_rows -1){
            for(int i = 0; i < n_cols; i++){
                if (tmp_row[i] == 1){
                    starting_x[0] = i;
                    break;
                }
            }
            for(int i = n_cols -1; i >=0; i--){
                if (tmp_row[i] == 1){
                    starting_x[1] = i;
                    break;
                }
            }
        }
    }
    
    // The finish line is on the right most columns.
    // Read the top and bottom of the finish line, store
    // them on ending_y[0] and ending_y[1]
    for(int i = 0; i < n_rows; i++){
        if (track[i][n_cols-1] == 1){
            ending_y[0] = i;
            break;
        }
    }
    for(int i = n_rows -1; i >=0 ; i--){
        if (track[i][n_cols-1] == 1){
            ending_y[1] = i;
            break;
        }
    }

    // Initialize the Action [n_row, n_cols, vy, vx]
    policy = int4(n_rows,
             int3(n_cols,
             int2(max_speed,
             int1(max_speed))));

    // Initialize the Q table: [n_row, n_cols, vy, vx, action]
    Q = double5(n_rows, 
        double4(n_cols, 
        double3(max_speed, 
        double2(max_speed, 
        double1(n_action)))));
    int tmp_max_action = 0;
    double tmp_max_q = 0.0;
    for(int i = 0; i < n_rows; i++)
        for(int j =0; j < n_cols; j++)
            for(int k = 0; k < max_speed; k++)
                for(int l = 0; l < max_speed; l++){
                    tmp_max_action = 0;
                    tmp_max_q = -100000;
                    for(int n = 0;n < n_action; n++){
                        // Important: As the game reward negative scores,
                        // cell that has not been visit must be initialized
                        // with large negative scores. Otherwise, 
                        // it would take really long to converg to optimal policy.
                        // You can try this by remove the negative sign from the
                        // equation below and print out the number of 
                        // steps in each episode
                        Q[i][j][k][l][n] = -100.0*(rand() % 100000) / 100000;
                        if (Q[i][j][k][l][n] > tmp_max_q){
                            tmp_max_q = Q[i][j][k][l][n];
                            tmp_max_action = n;
                        }
                    }

                    // Set the policy based on the initialized action value
                    policy[i][j][k][l] = tmp_max_action;
                }
    // Initialize the Counter [n_rows, n_cols, vy, vx, n_action]
    C = double5(n_rows,
        double4(n_cols,
        double3(max_speed,
        double2(max_speed,
        double1(n_action, 0.0)))));
}

int main(){
    srand(time(NULL));
    RaceCar racecar("data/track2.json");
    
    int n = 10000000;
    for(int i = 0; i < n; i++){
        racecar.episode();
        if (i % 5000 == 0) cout << "step: " << i << endl;
    }
    racecar.get_optimal_policy_track("data/track_2_policy.json");
}
