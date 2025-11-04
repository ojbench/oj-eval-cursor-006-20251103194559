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
  // Multi-pass strategy with iterative reasoning
  bool found_action = true;
  
  while (found_action) {
    found_action = false;
    
    // Pass 1: Find cells we can safely mark as mines
    // If a numbered cell has N unknown neighbors and needs N more mines, mark all unknown as mines
    for (int i = 0; i < rows && !found_action; i++) {
      for (int j = 0; j < columns && !found_action; j++) {
        if (is_visited_cell[i][j] && map_state[i][j] >= '0' && map_state[i][j] <= '8') {
          int mine_count = map_state[i][j] - '0';
          int remaining_mines = mine_count - marked_neighbors[i][j];
          
          if (remaining_mines > 0 && remaining_mines == unknown_neighbors[i][j]) {
            // All unknown neighbors must be mines
            for (int di = -1; di <= 1 && !found_action; di++) {
              for (int dj = -1; dj <= 1 && !found_action; dj++) {
                if (di == 0 && dj == 0) continue;
                int ni = i + di, nj = j + dj;
                if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
                  Execute(ni, nj, 1);  // Mark as mine
                  return;
                }
              }
            }
          }
        }
      }
    }
    
    // Pass 2: Find cells we can safely visit
    // If a numbered cell has all its mines marked, we can safely visit all unknown neighbors
    for (int i = 0; i < rows && !found_action; i++) {
      for (int j = 0; j < columns && !found_action; j++) {
        if (is_visited_cell[i][j] && map_state[i][j] >= '0' && map_state[i][j] <= '8') {
          int mine_count = map_state[i][j] - '0';
          
          if (mine_count == marked_neighbors[i][j] && unknown_neighbors[i][j] > 0) {
            // All mines found, safe to visit unknown neighbors
            for (int di = -1; di <= 1 && !found_action; di++) {
              for (int dj = -1; dj <= 1 && !found_action; dj++) {
                if (di == 0 && dj == 0) continue;
                int ni = i + di, nj = j + dj;
                if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && map_state[ni][nj] == '?') {
                  Execute(ni, nj, 0);  // Visit
                  return;
                }
              }
            }
          }
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
  
  // Step 4: Advanced probability-based guessing
  // Find the cell with lowest probability of being a mine
  int best_i = -1, best_j = -1;
  double min_mine_prob = 2.0;
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (map_state[i][j] == '?') {
        // Calculate mine probability based on neighbors
        int total_weight = 0;
        int mine_weight = 0;
        
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
                  total_weight++;
                  mine_weight += remaining;
                }
              }
            }
          }
        }
        
        double prob = (total_weight > 0) ? (double)mine_weight / total_weight : 0.5;
        
        // Prefer corners and edges
        bool is_corner = (i == 0 || i == rows-1) && (j == 0 || j == columns-1);
        bool is_edge = (i == 0 || i == rows-1 || j == 0 || j == columns-1);
        
        if (is_corner) prob *= 0.7;
        else if (is_edge) prob *= 0.85;
        
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
  
  // Last resort: visit any unknown cell
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (map_state[i][j] == '?') {
        Execute(i, j, 0);
        return;
      }
    }
  }
}

#endif