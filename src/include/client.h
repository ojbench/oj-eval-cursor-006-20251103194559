#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Custom global variables for client state
char map_state[35][35];  // Current state of each cell ('?', '0'-'8', '@', 'X')
bool is_known_mine[35][35];  // Whether we know a cell is a mine
bool is_visited_cell[35][35];  // Whether a cell has been visited
int unknown_neighbors[35][35];  // Count of unknown neighbors for each cell
int marked_neighbors[35][35];   // Count of marked neighbors for each cell
bool is_safe[35][35];  // Cells we've determined are safe
bool needs_marking[35][35];  // Cells we've determined are mines

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all custom global variables
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      map_state[i][j] = '?';
      is_known_mine[i][j] = false;
      is_visited_cell[i][j] = false;
      unknown_neighbors[i][j] = 0;
      marked_neighbors[i][j] = 0;
      is_safe[i][j] = false;
      needs_marking[i][j] = false;
    }
  }
  
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  // Read the map and update our state
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      map_state[i][j] = line[j];
      
      // Update visited status
      if (line[j] >= '0' && line[j] <= '8') {
        is_visited_cell[i][j] = true;
      } else if (line[j] == '@') {
        is_known_mine[i][j] = true;
      } else if (line[j] == 'X') {
        // Hit a mine or marked wrong
        is_visited_cell[i][j] = true;
      }
    }
  }
  
  // Update neighbor counts for all visited cells
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (is_visited_cell[i][j] && map_state[i][j] >= '0' && map_state[i][j] <= '8') {
        int unknown = 0, marked = 0;
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di, nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (map_state[ni][nj] == '?') {
                unknown++;
              } else if (map_state[ni][nj] == '@') {
                marked++;
              }
            }
          }
        }
        unknown_neighbors[i][j] = unknown;
        marked_neighbors[i][j] = marked;
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // Advanced constraint solving with multiple passes
  bool made_progress = true;
  int max_iterations = 10;
  
  while (made_progress && max_iterations-- > 0) {
    made_progress = false;
    
    // Reset deduction arrays
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        is_safe[i][j] = false;
        needs_marking[i][j] = false;
      }
    }
    
    // Phase 1: Single-cell constraint analysis
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (is_visited_cell[i][j] && map_state[i][j] >= '0' && map_state[i][j] <= '8') {
          int mine_count = map_state[i][j] - '0';
          int remaining_mines = mine_count - marked_neighbors[i][j];
          
          if (remaining_mines > 0 && remaining_mines == unknown_neighbors[i][j]) {
            // All unknown neighbors must be mines
            for (int di = -1; di <= 1; di++) {
              for (int dj = -1; dj <= 1; dj++) {
                if (di == 0 && dj == 0) continue;
                int ni = i + di, nj = j + dj;
                if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
                  needs_marking[ni][nj] = true;
                  made_progress = true;
                }
              }
            }
          } else if (mine_count == marked_neighbors[i][j] && unknown_neighbors[i][j] > 0) {
            // All mines found, safe to visit unknown neighbors
            for (int di = -1; di <= 1; di++) {
              for (int dj = -1; dj <= 1; dj++) {
                if (di == 0 && dj == 0) continue;
                int ni = i + di, nj = j + dj;
                if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
                  is_safe[ni][nj] = true;
                  made_progress = true;
                }
              }
            }
          }
        }
      }
    }
    
    // Phase 2: Pattern matching and additional constraint propagation
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (is_visited_cell[i][j] && map_state[i][j] >= '1' && map_state[i][j] <= '8') {
          int mc = map_state[i][j] - '0';
          int rem = mc - marked_neighbors[i][j];
          
          // Pattern: 1-2-1 horizontal
          if (mc == 1 && j + 2 < columns && is_visited_cell[i][j+2] && map_state[i][j+2] == '1' &&
              is_visited_cell[i][j+1] && map_state[i][j+1] == '2') {
            if (i > 0 && map_state[i-1][j+1] == '?') is_safe[i-1][j+1] = true;
            if (i < rows-1 && map_state[i+1][j+1] == '?') is_safe[i+1][j+1] = true;
          }
          // Pattern: 1-2-1 vertical
          if (mc == 1 && i + 2 < rows && is_visited_cell[i+2][j] && map_state[i+2][j] == '1' &&
              is_visited_cell[i+1][j] && map_state[i+1][j] == '2') {
            if (j > 0 && map_state[i+1][j-1] == '?') is_safe[i+1][j-1] = true;
            if (j < columns-1 && map_state[i+1][j+1] == '?') is_safe[i+1][j+1] = true;
          }
          
          // Pattern: 1-2 edge  configurations
          if (mc == 2 && rem > 0) {
            // Check for adjacent 1s that share unknowns
            for (int di = -1; di <= 1; di++) {
              for (int dj = -1; dj <= 1; dj++) {
                if (di == 0 && dj == 0) continue;
                int ni = i + di, nj = j + dj;
                if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
                  if (is_visited_cell[ni][nj] && map_state[ni][nj] == '1') {
                    int rem_neighbor = 1 - marked_neighbors[ni][nj];
                    if (rem_neighbor == 1) {
                      // The 1 needs exactly 1 mine, the 2 needs rem mines
                      // Check if they share unknowns
                      int shared_unknowns = 0;
                      for (int di2 = -1; di2 <= 1; di2++) {
                        for (int dj2 = -1; dj2 <= 1; dj2++) {
                          if (di2 == 0 && dj2 == 0) continue;
                          int ci = i + di2, cj = j + dj2;
                          if (ci >= 0 && ci < rows && cj >= 0 && cj < columns && map_state[ci][cj] == '?') {
                            // Check if also neighbor of the 1
                            if (abs(ci - ni) <= 1 && abs(cj - nj) <= 1) {
                              shared_unknowns++;
                            }
                          }
                        }
                      }
                      // Apply deduction if pattern matches
                      if (shared_unknowns > 0 && unknown_neighbors[ni][nj] == shared_unknowns) {
                        // The 1's mine must be in shared area
                        // So (2's unknowns - shared) can have at most (rem-1) mines
                        if (rem == 1) {
                          // All of 2's unique unknowns are safe
                          for (int di2 = -1; di2 <= 1; di2++) {
                            for (int dj2 = -1; dj2 <= 1; dj2++) {
                              if (di2 == 0 && dj2 == 0) continue;
                              int ci = i + di2, cj = j + dj2;
                              if (ci >= 0 && ci < rows && cj >= 0 && cj < columns && map_state[ci][cj] == '?') {
                                if (abs(ci - ni) > 1 || abs(cj - nj) > 1) {
                                  is_safe[ci][cj] = true;
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    
    // Execute marked mines first
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (needs_marking[i][j] && map_state[i][j] == '?') {
          Execute(i, j, 1);
          return;
        }
      }
    }
    
    // Execute safe cells
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (is_safe[i][j] && map_state[i][j] == '?') {
          Execute(i, j, 0);
          return;
        }
      }
    }
  }
  
  // Step 3: Simple subset constraint deduction
  // Look for pairs of cells where one's unknowns are subset of another's
  for (int i1 = 0; i1 < rows; i1++) {
    for (int j1 = 0; j1 < columns; j1++) {
      if (!is_visited_cell[i1][j1] || map_state[i1][j1] < '1' || map_state[i1][j1] > '8') continue;
      
      int mc1 = map_state[i1][j1] - '0';
      int rem1 = mc1 - marked_neighbors[i1][j1];
      if (rem1 <= 0) continue;
      
      // Get unknown neighbors of cell1
      int unknowns1[8][2], unk1_count = 0;
      for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
          if (di == 0 && dj == 0) continue;
          int ni = i1 + di, nj = j1 + dj;
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
            unknowns1[unk1_count][0] = ni;
            unknowns1[unk1_count][1] = nj;
            unk1_count++;
          }
        }
      }
      
      // Check nearby cells
      for (int i2 = i1 - 2; i2 <= i1 + 2 && i2 < rows; i2++) {
        for (int j2 = j1 - 2; j2 <= j1 + 2 && j2 < columns; j2++) {
          if (i2 < 0 || (i2 == i1 && j2 == j1)) continue;
          if (!is_visited_cell[i2][j2] || map_state[i2][j2] < '1' || map_state[i2][j2] > '8') continue;
          
          int mc2 = map_state[i2][j2] - '0';
          int rem2 = mc2 - marked_neighbors[i2][j2];
          if (rem2 <= 0) continue;
          
          // Get unknown neighbors of cell2
          int unknowns2[8][2], unk2_count = 0;
          for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
              if (di == 0 && dj == 0) continue;
              int ni = i2 + di, nj = j2 + dj;
              if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
                unknowns2[unk2_count][0] = ni;
                unknowns2[unk2_count][1] = nj;
                unk2_count++;
              }
            }
          }
          
          // Check if unknowns1 is subset of unknowns2
          bool is_subset = true;
          for (int k = 0; k < unk1_count; k++) {
            bool found = false;
            for (int m = 0; m < unk2_count; m++) {
              if (unknowns1[k][0] == unknowns2[m][0] && unknowns1[k][1] == unknowns2[m][1]) {
                found = true;
                break;
              }
            }
            if (!found) {
              is_subset = false;
              break;
            }
          }
          
          if (is_subset && unk1_count < unk2_count) {
            // unknowns1 ? unknowns2, so unknowns2 - unknowns1 has (rem2 - rem1) mines
            int diff_mines = rem2 - rem1;
            int diff_count = unk2_count - unk1_count;
            
            if (diff_mines == diff_count && diff_count > 0) {
              // All cells in (unknowns2 - unknowns1) are mines
              for (int m = 0; m < unk2_count; m++) {
                bool in_both = false;
                for (int k = 0; k < unk1_count; k++) {
                  if (unknowns2[m][0] == unknowns1[k][0] && unknowns2[m][1] == unknowns1[k][1]) {
                    in_both = true;
                    break;
                  }
                }
                if (!in_both) {
                  Execute(unknowns2[m][0], unknowns2[m][1], 1);
                  return;
                }
              }
            } else if (diff_mines == 0 && diff_count > 0) {
              // All cells in (unknowns2 - unknowns1) are safe
              for (int m = 0; m < unk2_count; m++) {
                bool in_both = false;
                for (int k = 0; k < unk1_count; k++) {
                  if (unknowns2[m][0] == unknowns1[k][0] && unknowns2[m][1] == unknowns1[k][1]) {
                    in_both = true;
                    break;
                  }
                }
                if (!in_both) {
                  Execute(unknowns2[m][0], unknowns2[m][1], 0);
                  return;
                }
              }
            }
          }
        }
      }
    }
  }
  
  // Step 4: Try auto-explore on cells where all mines are marked
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (is_visited_cell[i][j] && map_state[i][j] >= '0' && map_state[i][j] <= '8') {
        int mine_count = map_state[i][j] - '0';
        if (mine_count > 0 && mine_count == marked_neighbors[i][j] && unknown_neighbors[i][j] > 0) {
          Execute(i, j, 2);  // Auto-explore
          return;
        }
      }
    }
  }
  
  // Step 4: Improved probability-based guessing
  // Calculate mine probability for each unknown cell more accurately
  int best_i = -1, best_j = -1;
  double min_mine_prob = 2.0;
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (map_state[i][j] == '?') {
        // Calculate mine probability using average of individual probabilities
        double prob_sum = 0.0;
        int constraint_count = 0;
        
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di, nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (is_visited_cell[ni][nj] && map_state[ni][nj] >= '0' && map_state[ni][nj] <= '8') {
                int mc = map_state[ni][nj] - '0';
                int remaining = mc - marked_neighbors[ni][nj];
                int unknown = unknown_neighbors[ni][nj];
                
                if (unknown > 0) {
                  // Each constraint gives an independent probability estimate
                  prob_sum += (double)remaining / unknown;
                  constraint_count++;
                }
              }
            }
          }
        }
        
        // Average the probability estimates
        double prob = (constraint_count > 0) ? prob_sum / constraint_count : 0.5;
        
        // Apply heuristics: prefer corners and edges (statistically safer)
        bool is_corner = (i == 0 || i == rows-1) && (j == 0 || j == columns-1);
        bool is_edge = (i == 0 || i == rows-1 || j == 0 || j == columns-1);
        
        // Stronger preference for cells with no constraints (far from frontier)
        if (constraint_count == 0) {
          prob = 0.3;  // Assume lower mine density in unexplored areas
        }
        
        if (is_corner) prob *= 0.65;
        else if (is_edge) prob *= 0.80;
        
        // Prefer cells with more constraints (more information)
        double confidence_bonus = 1.0 / (1.0 + constraint_count * 0.1);
        prob *= confidence_bonus;
        
        if (prob < min_mine_prob) {
          min_mine_prob = prob;
          best_i = i;
          best_j = j;
        }
      }
    }
  }
  
  if (best_i != -1) {
    Execute(best_i, best_j, 0);
    return;
  }
}

#endif