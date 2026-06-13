#include "game_logic.h"

#include <algorithm>
#include <iostream>

#include "csv_loader.h"
#include "display.h"

static std::vector<IngredientType> StorageForFloor(FloorLevel level) {
  if (level == FLOOR_1) {
    return {COFFEE_BEANS, SUGAR, BREAD,  HAM,    LETTUCE, TOMATO,
            MAYO,         ONION, CHEESE, GARLIC, BEER,    CHOCOLATE};
  } else if (level == FLOOR_2) {
    return {COFFEE_BEANS, SUGAR, BREAD,  HAM,    LETTUCE, TOMATO,
            MAYO,         ONION, CHEESE, GARLIC, BEER,    CHOCOLATE,
            DOUGH,        MEAT,  FISH,   RICE,   BUTTER};
  } else {
    return {COFFEE_BEANS, SUGAR,  BREAD,  HAM,  LETTUCE,   TOMATO, MAYO,
            ONION,        CHEESE, GARLIC, BEER, CHOCOLATE, DOUGH,  MEAT,
            FISH,         RICE,   BUTTER, WINE, BEETROOT};
  }
}

Floor BuildFloor(FloorLevel level, const std::vector<Recipe>& all_recipes,
                 const std::vector<Customer>& all_customers,
                 const GameConfig& config) {
  Floor floor;
  floor.level = level;
  floor.served_count = 0;
  floor.is_cleared = false;
  floor.storage_items = StorageForFloor(level);

  int floor_num = static_cast<int>(level);
  for (const Recipe& recipe : all_recipes) {
    if (recipe.floor_found == 0 || recipe.floor_found <= floor_num) {
      floor.available_recipes.push_back(recipe);
    }
  }

  for (const Customer& customer : all_customers) {
    if (customer.floor == floor_num) {
      floor.customers.push_back(customer);
    }
  }

  if (level == FLOOR_1)
    floor.min_to_progress = config.floor1_min_to_progress;
  else if (level == FLOOR_2)
    floor.min_to_progress = config.floor2_min_to_progress;
  else
    floor.min_to_progress = config.floor3_min_to_progress;

  return floor;
}

GameState InitGame(const std::string& data_path) {
  GameState state;
  state.is_running = true;
  state.ending = ENDING_NONE;
  state.current_floor = FLOOR_1;
  state.escape_key_available = false;
  state.all_floors_cleared = false;

  state.config = LoadConfig(data_path + "/config.csv");
  std::vector<Recipe> all_recipes = LoadRecipes(data_path + "/recipes.csv");
  std::vector<Customer> all_customers =
      LoadCustomers(data_path + "/customers.csv", state.config);

  state.floors.push_back(
      BuildFloor(FLOOR_1, all_recipes, all_customers, state.config));
  state.floors.push_back(
      BuildFloor(FLOOR_2, all_recipes, all_customers, state.config));
  state.floors.push_back(
      BuildFloor(FLOOR_3, all_recipes, all_customers, state.config));

  state.player.max_inventory = state.config.inventory_size;
  state.player.has_key = false;
  state.player.upgrade_points = 0;
  state.player.customers_served_total = 0;
  state.player.has_memory_item = false;
  state.player.has_truffle_item = false;
  state.player.has_spice_item = false;

  return state;
}

bool AddToInventory(Player& player, IngredientType item) {
  if (static_cast<int>(player.inventory.size()) >= player.max_inventory) {
    return false;
  }
  player.inventory.push_back(item);
  return true;
}

void RemoveFromInventory(Player& player, int slot_index) {
  if (slot_index < 0 || slot_index >= static_cast<int>(player.inventory.size()))
    return;
  player.inventory.erase(player.inventory.begin() + slot_index);
}

bool HasIngredients(const Player& player, const Recipe& recipe) {
  std::vector<IngredientType> inventory_copy = player.inventory;
  for (IngredientType ingredient : recipe.ingredients) {
    auto it =
        std::find(inventory_copy.begin(), inventory_copy.end(), ingredient);
    if (it == inventory_copy.end()) return false;
    inventory_copy.erase(it);
  }
  return true;
}

void ConsumeIngredients(Player& player, const Recipe& recipe) {
  for (IngredientType ingredient : recipe.ingredients) {
    auto it =
        std::find(player.inventory.begin(), player.inventory.end(), ingredient);
    if (it != player.inventory.end()) {
      player.inventory.erase(it);
    }
  }
}

bool CanProgressToNextFloor(const Floor& floor) {
  return floor.served_count >= floor.min_to_progress;
}

bool CheckLossCondition(const GameState& state) {
  for (const Floor& floor : state.floors) {
    if (floor.is_cleared) continue;
    int remaining_active = 0;
    for (const Customer& customer : floor.customers) {
      if (!customer.is_served && !customer.has_left) ++remaining_active;
    }
    int possible = floor.served_count + remaining_active;
    if (possible < floor.min_to_progress) return true;
  }
  return false;
}

void CheckAndAwardUpgrade(GameState& state) {
  int cost = state.config.upgrade_cost;
  while (state.player.upgrade_points >= cost) {
    int floor_index = static_cast<int>(state.current_floor) - 1;
    Floor& floor = state.floors[floor_index];
    state.player.upgrade_points -= cost;

    ShowUpgradeMenu(floor.available_recipes);
    int choice;
    std::cin >> choice;
    std::cin.ignore(1000, '\n');

    if (choice == 0) {
      state.player.upgrade_points += cost;
      break;
    }
    --choice;
    if (choice >= 0 &&
        choice < static_cast<int>(floor.available_recipes.size())) {
      Recipe& recipe = floor.available_recipes[choice];
      if (recipe.ingredients.size() >= 2) {
        recipe.ingredients.pop_back();
        std::cout << "\nРецепт «" << recipe.name
                  << "» улучшен! Теперь нужно на 1 ингредиент меньше.\n";
        PressEnter();
      } else {
        std::cout << "\nЭтот рецепт уже минимален.\n";
        state.player.upgrade_points += cost;
        PressEnter();
      }
    } else {
      state.player.upgrade_points += cost;
      break;
    }
  }
}

void CheckStoryTriggers(GameState& state, Floor& floor, int customer_index) {
  if (floor.level == FLOOR_2 && !state.player.has_truffle_item &&
      floor.served_count == 1) {
    state.player.has_truffle_item = true;
    ShowStoryEvent(
        "Убирая на складе после первой группы, ты замечаешь в дальнем углу\n"
        "нечто необычное — трюфель в окружении сыра, плесени и грибка.\n\n"
        "Странно. Отвратительно. Но что-то внутри говорит: сохрани.\n\n"
        "Ты кладёшь находку в карман. «Интересный ингредиент» для будущего "
        "рецепта.");
  }
  if (floor.level == FLOOR_3 && !state.player.has_spice_item &&
      floor.served_count == 1) {
    state.player.has_spice_item = true;
    ShowStoryEvent(
        "Первый VIP-гость едва улыбнулся. Остальные смотрят на тебя\n"
        "как на прислугу. Злость поднимается внутри — горячая, жгучая.\n\n"
        "Стоп. Это чувство... В идеальном блюде нужна перчинка.\n"
        "Лёгкость после остроты. Вот оно — третий компонент рецепта.\n\n"
        "«Изюминка с перчинкой» теперь у тебя.");
  }
  if (floor.level == FLOOR_2 &&
      floor.customers[customer_index].is_escape_trigger &&
      floor.customers[customer_index].is_served) {
    state.escape_key_available = true;
    state.player.has_key = true;
    ShowStoryEvent(
        "Семья за столом смотрит на тебя с теплом. Отец семейства — ты "
        "узнаёшь\n"
        "его. Бывший повар этого ресторана. Он понимает твой взгляд.\n\n"
        "«Я тоже был здесь. Шеф держал нас всех. Возьми — тебе пригодится.»\n\n"
        "Он протягивает старый ключ. Ключ от чёрного хода.\n"
        "Ты можешь уйти прямо сейчас.");
  }
}

Ending DetermineEnding(const GameState& state) {
  bool has_final_recipe = state.player.has_memory_item &&
                          state.player.has_truffle_item &&
                          state.player.has_spice_item;
  if (!has_final_recipe) return ENDING_TRAPPED;

  bool all_served = true;
  for (const Floor& floor : state.floors) {
    if (floor.served_count < static_cast<int>(floor.customers.size())) {
      all_served = false;
      break;
    }
  }
  if (all_served) return ENDING_DECISION;

  return ENDING_HONEST;
}
