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
    
    // Phase 2: Pattern matching for common configurations
    // Pattern: 1-2-1 or 1-1 patterns
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (is_visited_cell[i][j] && map_state[i][j] == '1') {
          // Check for 1-2-1 pattern horizontally
          if (j + 2 < columns && is_visited_cell[i][j+2] && map_state[i][j+2] == '1' &&
              is_visited_cell[i][j+1] && map_state[i][j+1] == '2') {
            // Middle cell neighbors (not shared with edges) are safe
            if (i > 0 && map_state[i-1][j+1] == '?') is_safe[i-1][j+1] = true;
            if (i < rows-1 && map_state[i+1][j+1] == '?') is_safe[i+1][j+1] = true;
          }
          // Check for 1-2-1 pattern vertically
          if (i + 2 < rows && is_visited_cell[i+2][j] && map_state[i+2][j] == '1' &&
              is_visited_cell[i+1][j] && map_state[i+1][j] == '2') {
            if (j > 0 && map_state[i+1][j-1] == '?') is_safe[i+1][j-1] = true;
            if (j < columns-1 && map_state[i+1][j+1] == '?') is_safe[i+1][j+1] = true;
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
  
  // Step 3: Try auto-explore on cells where all mines are marked
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
  
  // Step 4: Subset constraint solving
  // Look for overlapping constraints that can deduce mine locations
  for (int i1 = 0; i1 < rows; i1++) {
    for (int j1 = 0; j1 < columns; j1++) {
      if (!is_visited_cell[i1][j1] || map_state[i1][j1] < '1' || map_state[i1][j1] > '8') continue;
      
      int mc1 = map_state[i1][j1] - '0';
      int rem1 = mc1 - marked_neighbors[i1][j1];
      if (rem1 <= 0 || unknown_neighbors[i1][j1] <= 0) continue;
      
      // Check for nearby cells with overlapping unknown neighbors
      for (int i2 = i1 - 2; i2 <= i1 + 2; i2++) {
        for (int j2 = j1 - 2; j2 <= j1 + 2; j2++) {
          if (i2 < 0 || i2 >= rows || j2 < 0 || j2 >= columns) continue;
          if (i2 == i1 && j2 == j1) continue;
          if (!is_visited_cell[i2][j2] || map_state[i2][j2] < '1' || map_state[i2][j2] > '8') continue;
          
          int mc2 = map_state[i2][j2] - '0';
          int rem2 = mc2 - marked_neighbors[i2][j2];
          if (rem2 <= 0 || unknown_neighbors[i2][j2] <= 0) continue;
          
          // Find shared and unique unknown neighbors
          int shared = 0, unique1 = 0, unique2 = 0;
          bool has_unique1[8] = {false}, has_unique2[8] = {false};
          int unique1_pos[8][2], unique2_pos[8][2];
          
          for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
              if (di == 0 && dj == 0) continue;
              int ni = i1 + di, nj = j1 + dj;
              if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
                bool is_neighbor2 = (abs(ni - i2) <= 1 && abs(nj - j2) <= 1);
                if (is_neighbor2) {
                  shared++;
                } else {
                  unique1_pos[unique1][0] = ni;
                  unique1_pos[unique1][1] = nj;
                  unique1++;
                }
              }
            }
          }
          
          for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
              if (di == 0 && dj == 0) continue;
              int ni = i2 + di, nj = j2 + dj;
              if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
                bool is_neighbor1 = (abs(ni - i1) <= 1 && abs(nj - j1) <= 1);
                if (!is_neighbor1) {
                  unique2_pos[unique2][0] = ni;
                  unique2_pos[unique2][1] = nj;
                  unique2++;
                }
              }
            }
          }
          
          // Check if cell1's constraint minus shared equals unique1 (all unique1 are mines)
          if (rem1 == unique1 + shared && unique1 > 0 && rem2 <= shared) {
            for (int k = 0; k < unique1; k++) {
              int ni = unique1_pos[k][0], nj = unique1_pos[k][1];
              Execute(ni, nj, 1);
              return;
            }
          }
          
          // Check if all of cell1's mines are in shared (unique2 are safe)
          if (rem1 <= shared && unique2 > 0) {
            for (int k = 0; k < unique2; k++) {
              int ni = unique2_pos[k][0], nj = unique2_pos[k][1];
              Execute(ni, nj, 0);
              return;
            }
          }
        }
      }
    }
  }
  
  // Step 5: Probability-based guessing with improved heuristics
  int best_i = -1, best_j = -1;
  double min_mine_prob = 2.0;
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (map_state[i][j] == '?') {
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
                  prob_sum += (double)remaining / unknown;
                  constraint_count++;
                }
              }
            }
          }
        }
        
        double prob = (constraint_count > 0) ? prob_sum / constraint_count : 0.5;
        
        // Strong preference for corners and edges
        bool is_corner = (i == 0 || i == rows-1) && (j == 0 || j == columns-1);
        bool is_edge = (i == 0 || i == rows-1 || j == 0 || j == columns-1);
        
        if (is_corner) prob *= 0.6;
        else if (is_edge) prob *= 0.75;
        
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