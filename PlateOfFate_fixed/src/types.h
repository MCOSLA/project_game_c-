#pragma once

#include <string>
#include <vector>

enum IngredientType {
  NONE = -1,
  COFFEE_BEANS = 0,
  SUGAR,
  BREAD,
  HAM,
  LETTUCE,
  TOMATO,
  MAYO,
  ONION,
  CHEESE,
  GARLIC,
  BEER,
  CHOCOLATE,
  DOUGH,
  MEAT,
  FISH,
  RICE,
  BUTTER,
  WINE,
  BEETROOT,
  INGREDIENT_COUNT
};

enum FloorLevel { FLOOR_1 = 1, FLOOR_2 = 2, FLOOR_3 = 3 };

enum Ending {
  ENDING_NONE,
  ENDING_HONEST,
  ENDING_DECISION,
  ENDING_ESCAPE,
  ENDING_TRAPPED
};

enum CustomerType { CUSTOMER_SINGLE, CUSTOMER_GROUP, CUSTOMER_VIP };

struct Recipe {
  int id;
  std::string name;
  std::vector<IngredientType> ingredients;
  int floor_found;
};

struct Customer {
  int id;
  int floor;
  CustomerType type;
  std::string description;
  std::string hint;
  int correct_recipe_id;
  bool is_escape_trigger;
  int patience;
  bool is_served;
  bool has_left;
};

struct Player {
  std::vector<IngredientType> inventory;
  int max_inventory;
  bool has_key;
  int upgrade_points;
  int customers_served_total;
  bool has_memory_item;
  bool has_truffle_item;
  bool has_spice_item;
};

struct Floor {
  FloorLevel level;
  std::vector<IngredientType> storage_items;
  std::vector<Recipe> available_recipes;
  std::vector<Customer> customers;
  int min_to_progress;
  int served_count;
  bool is_cleared;
};

struct GameConfig {
  int floor1_total_customers;
  int floor1_min_to_progress;
  int floor1_patience;
  int floor2_total_groups;
  int floor2_min_to_progress;
  int floor2_patience;
  int floor2_group_size;
  int floor3_total_vip;
  int floor3_min_to_progress;
  int floor3_patience;
  int inventory_size;
  int upgrade_cost;
  int points_per_customer;
};

struct GameState {
  FloorLevel current_floor;
  bool is_running;
  Ending ending;
  Player player;
  std::vector<Floor> floors;
  GameConfig config;
  bool escape_key_available;
  bool all_floors_cleared;
};
