#include <windows.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "csv_loader.h"
#include "display.h"
#include "game_logic.h"
#include "types.h"

static int ReadInt(int min_val, int max_val) {
  int value;
  while (true) {
    if (std::cin >> value) {
      std::cin.ignore(1000, '\n');
      if (value >= min_val && value <= max_val) return value;
    } else {
      std::cin.clear();
      std::cin.ignore(1000, '\n');
    }
    std::cout << "Введите число от " << min_val << " до " << max_val << ": ";
  }
}

static void HandleStorage(GameState& state) {
  Floor& floor = state.floors[static_cast<int>(state.current_floor) - 1];
  while (true) {
    ShowStorageMenu(floor, state.player, floor.available_recipes);
    std::cout << "Действия:\n";
    std::cout << "  1-" << floor.storage_items.size() << ". Взять ингредиент\n";
    std::cout << "  D. Выбросить предмет из инвентаря\n";
    std::cout << "  0. Уйти со склада\n\n";
    std::cout << "Ваш выбор: ";

    std::string input;
    std::getline(std::cin, input);

    if (input == "0") break;

    if (input == "D" || input == "d" || input == "д" || input == "Д") {
      if (state.player.inventory.empty()) {
        std::cout << "Инвентарь пуст.\n";
        PressEnter();
        continue;
      }
      std::cout << "Выберите предмет для выброса (1-"
                << state.player.inventory.size() << "): ";
      int slot =
          ReadInt(1, static_cast<int>(state.player.inventory.size())) - 1;
      IngredientType dropped = state.player.inventory[slot];
      RemoveFromInventory(state.player, slot);
      if (IsCookedDish(dropped)) {
        std::cout << "Вы выбросили: [БЛЮДО] "
                  << CookedDishName(dropped, floor.available_recipes) << "\n";
      } else {
        std::cout << "Вы выбросили: " << IngredientToString(dropped) << "\n";
      }
      PressEnter();
      continue;
    }

    try {
      int choice = std::stoi(input) - 1;
      if (choice < 0 ||
          choice >= static_cast<int>(floor.storage_items.size())) {
        std::cout << "Неверный выбор.\n";
        PressEnter();
        continue;
      }
      IngredientType item = floor.storage_items[choice];
      if (!AddToInventory(state.player, item)) {
        std::cout << "Инвентарь полон! Выбросьте что-нибудь (D).\n";
        PressEnter();
      } else {
        std::cout << "Взято: " << IngredientToString(item) << "\n";
        PressEnter();
      }
    } catch (...) {
      std::cout << "Неверный ввод.\n";
      PressEnter();
    }
  }
}

static int HandleKitchen(GameState& state) {
  Floor& floor = state.floors[static_cast<int>(state.current_floor) - 1];
  while (true) {
    ShowKitchenMenu(floor, state.player, floor.available_recipes);
    std::cout << "Действия:\n";
    std::cout << "  1-" << floor.available_recipes.size()
              << ". Приготовить блюдо\n";
    std::cout << "  B. Книга рецептов\n";
    std::cout << "  0. Уйти с кухни\n\n";
    std::cout << "Ваш выбор: ";

    std::string input;
    std::getline(std::cin, input);

    if (input == "0") return -1;
    if (input == "B" || input == "b" || input == "б" || input == "Б") {
      ShowRecipeBook(floor.available_recipes);
      PressEnter();
      continue;
    }

    try {
      int choice = std::stoi(input) - 1;
      if (choice < 0 ||
          choice >= static_cast<int>(floor.available_recipes.size())) {
        std::cout << "Неверный выбор.\n";
        PressEnter();
        continue;
      }
      const Recipe& recipe = floor.available_recipes[choice];
      if (!HasIngredients(state.player, recipe)) {
        std::cout << "\nНедостаточно ингредиентов!\n";
        std::cout << "Нужно: " << RecipeIngredientsList(recipe) << "\n";
        std::cout << "Идите на склад за ингредиентами.\n";
        PressEnter();
        continue;
      }
      ConsumeIngredients(state.player, recipe);
      IngredientType token = static_cast<IngredientType>(
          static_cast<int>(INGREDIENT_COUNT) + recipe.id);
      if (!AddToInventory(state.player, token)) {
        std::cout << "\nИнвентарь полон! Выбросьте что-нибудь на складе.\n";
        for (IngredientType ingredient : recipe.ingredients) {
          AddToInventory(state.player, ingredient);
        }
        PressEnter();
        continue;
      }
      std::cout << "\n✓ Приготовлено: «" << recipe.name << "»!\n";
      PressEnter();
      return recipe.id;
    } catch (...) {
      std::cout << "Неверный ввод.\n";
      PressEnter();
    }
  }
}

static bool HandleServeCustomer(GameState& state, Floor& floor,
                                int customer_index) {
  Customer& customer = floor.customers[customer_index];

  while (customer.patience > 0) {
    ClearScreen();
    ShowCustomerDialog(customer);

    int cooked_slot = -1;
    int cooked_recipe_id = -1;
    for (int i = 0; i < static_cast<int>(state.player.inventory.size()); ++i) {
      if (IsCookedDish(state.player.inventory[i])) {
        cooked_slot = i;
        cooked_recipe_id = CookedDishRecipeId(state.player.inventory[i]);
        break;
      }
    }

    std::cout << "--- Что делать? ---\n";
    std::cout << "  1. Выбрать блюдо из рецептов\n";
    if (cooked_slot >= 0) {
      std::string dish_name = CookedDishName(
          state.player.inventory[cooked_slot], floor.available_recipes);
      std::cout << "  2. Подать уже готовое: «" << dish_name << "»\n";
    }
    std::cout << "  3. Сначала сходить на склад\n";
    std::cout << "  4. Сначала сходить на кухню\n";
    std::cout << "  0. Пропустить клиента (клиент уйдёт)\n\n";
    std::cout << "Ваш выбор: ";

    int action = ReadInt(0, 4);

    if (action == 0) {
      customer.has_left = true;
      std::cout << "\nКлиент потерял терпение и ушёл.\n";
      PressEnter();
      return false;
    }
    if (action == 3) {
      HandleStorage(state);
      continue;
    }
    if (action == 4) {
      HandleKitchen(state);
      continue;
    }

    int served_recipe_id = -1;

    if (action == 2 && cooked_slot >= 0) {
      served_recipe_id = cooked_recipe_id;
      RemoveFromInventory(state.player, cooked_slot);
    } else {
      ClearScreen();
      ShowCustomerDialog(customer);
      ShowRecipeChoice(floor.available_recipes);
      int recipe_choice =
          ReadInt(0, static_cast<int>(floor.available_recipes.size()));
      if (recipe_choice == 0) continue;

      const Recipe& chosen_recipe = floor.available_recipes[recipe_choice - 1];

      bool found_cooked = false;
      for (int i = 0; i < static_cast<int>(state.player.inventory.size());
           ++i) {
        if (IsCookedDish(state.player.inventory[i]) &&
            CookedDishRecipeId(state.player.inventory[i]) == chosen_recipe.id) {
          RemoveFromInventory(state.player, i);
          found_cooked = true;
          served_recipe_id = chosen_recipe.id;
          break;
        }
      }
      if (!found_cooked) {
        if (!HasIngredients(state.player, chosen_recipe)) {
          std::cout << "\nУ вас нет ингредиентов для «" << chosen_recipe.name
                    << "».\n";
          std::cout << "Нужно: " << RecipeIngredientsList(chosen_recipe)
                    << "\n";
          std::cout << "Идите на склад, потом на кухню.\n";
          PressEnter();
          continue;
        }
        ConsumeIngredients(state.player, chosen_recipe);
        served_recipe_id = chosen_recipe.id;
        std::cout << "\nВы готовите «" << chosen_recipe.name << "»...\n";
      }
    }

    if (served_recipe_id == customer.correct_recipe_id) {
      customer.is_served = true;
      floor.served_count++;
      state.player.customers_served_total++;
      state.player.upgrade_points += state.config.points_per_customer;

      std::string recipe_name = "блюдо";
      for (const Recipe& recipe : floor.available_recipes) {
        if (recipe.id == served_recipe_id) {
          recipe_name = recipe.name;
          break;
        }
      }
      ClearScreen();
      std::cout << "\n★ Клиент доволен! «" << recipe_name
                << "» — именно то, что нужно!\n";
      std::cout << "+1 очко прокачки (всего: " << state.player.upgrade_points
                << ")\n\n";

      CheckStoryTriggers(state, floor, customer_index);
      CheckAndAwardUpgrade(state);
      PressEnter();
      return true;
    } else {
      customer.patience--;
      ClearScreen();
      std::cout << "\n✗ Неверно! Клиент недоволен.\n";
      std::cout << "Осталось терпения: ";
      for (int i = 0; i < customer.patience; ++i) std::cout << "♥ ";
      if (customer.patience == 0) std::cout << "(уходит)";
      std::cout << "\n\n";
      if (customer.patience == 0) {
        customer.has_left = true;
        std::cout << "Клиент потерял всё терпение и ушёл.\n";
        PressEnter();
        return false;
      }
      std::cout << "Клиент говорит: «Попробуйте ещё раз...»\n";
      PressEnter();
    }
  }

  customer.has_left = true;
  return false;
}

static void HandleHall(GameState& state) {
  Floor& floor = state.floors[static_cast<int>(state.current_floor) - 1];

  while (true) {
    ClearScreen();

    std::string floor_name;
    if (state.current_floor == FLOOR_1)
      floor_name = "ЗАЛ — Этаж 1";
    else if (state.current_floor == FLOOR_2)
      floor_name = "ЗАЛ — Этаж 2";
    else
      floor_name = "VIP ЗАЛ — Этаж 3";

    PrintTitle(floor_name);
    std::cout << "Обслужено: " << floor.served_count << "/"
              << floor.customers.size()
              << "  (минимум для перехода: " << floor.min_to_progress << ")\n";
    ShowInventory(state.player, floor.available_recipes);

    if (CheckLossCondition(state)) {
      std::cout
          << "\n[!] Слишком много клиентов ушло. Вы не выполните норму.\n";
      PressEnter();
      state.is_running = false;
      state.ending = ENDING_TRAPPED;
      return;
    }

    ShowCustomerList(floor);

    std::vector<int> active_indices;
    for (int i = 0; i < static_cast<int>(floor.customers.size()); ++i) {
      if (!floor.customers[i].is_served && !floor.customers[i].has_left) {
        active_indices.push_back(i);
      }
    }

    std::cout << "Действия:\n";
    if (!active_indices.empty()) {
      std::cout << "  1-" << active_indices.size() << ". Обслужить клиента\n";
    }
    std::cout << "  S. Склад\n";
    std::cout << "  K. Кухня\n";
    std::cout << "  R. Книга рецептов\n";

    bool can_escape = state.player.has_key && state.current_floor == FLOOR_2;
    if (can_escape) std::cout << "  E. Использовать ключ (ПОБЕГ)\n";

    bool can_next = CanProgressToNextFloor(floor);
    if (can_next && state.current_floor != FLOOR_3) {
      std::cout << "  N. Перейти на следующий этаж\n";
    }
    if (can_next && state.current_floor == FLOOR_3) {
      std::cout << "  N. Идти в офис шефа (финал)\n";
    }

    std::cout << "\nВаш выбор: ";
    std::string input;
    std::getline(std::cin, input);

    if (input == "S" || input == "s" || input == "с" || input == "С") {
      HandleStorage(state);
      continue;
    }
    if (input == "K" || input == "k" || input == "к" || input == "К") {
      HandleKitchen(state);
      continue;
    }
    if (input == "R" || input == "r" || input == "р" || input == "Р") {
      ShowRecipeBook(floor.available_recipes);
      PressEnter();
      continue;
    }
    if ((input == "E" || input == "e" || input == "е" || input == "Е") &&
        can_escape) {
      state.ending = ENDING_ESCAPE;
      state.is_running = false;
      return;
    }
    if ((input == "N" || input == "n" || input == "н" || input == "Н") &&
        can_next) {
      floor.is_cleared = true;
      return;
    }

    try {
      int choice = std::stoi(input) - 1;
      if (choice < 0 || choice >= static_cast<int>(active_indices.size())) {
        std::cout << "Неверный выбор.\n";
        PressEnter();
        continue;
      }
      int real_index = active_indices[choice];
      HandleServeCustomer(state, floor, real_index);
    } catch (...) {
      std::cout << "Неверный ввод.\n";
      PressEnter();
    }
  }
}

static void HandleBossOffice(GameState& state) {
  bool has_all_items = state.player.has_memory_item &&
                       state.player.has_truffle_item &&
                       state.player.has_spice_item;

  ClearScreen();
  PrintTitle("ОФИС ШЕФА");

  if (!has_all_items) {
    std::cout << "\nВы входите в офис, но рецепт ещё не готов.\n";
    std::cout << "Шеф смотрит на вас и молча рвёт ваше заявление.\n\n";
    state.ending = ENDING_TRAPPED;
    state.is_running = false;
    PressEnter();
    return;
  }

  ShowStoryEvent(
      "Вы входите в офис. В руках — последнее блюдо вашей жизни здесь.\n"
      "Три компонента нового рецепта: воспоминания, трюфель, перчинка.\n\n"
      "Шеф смотрит на тарелку. Долго. Молча.\n"
      "Потом берёт вилку.");

  Ending ending = DetermineEnding(state);
  state.ending = ending;
  state.is_running = false;

  if (ending == ENDING_DECISION) {
    ShowStoryEvent(
        "Вы накормили всех. Каждого клиента. Этаж за этажом.\n"
        "Ярость и усталость слились в одно чувство.\n\n"
        "Когда шеф открывает рот, чтобы сказать что-то унизительное —\n"
        "вы уже не слышите его.\n\n"
        "Тарелка летит.\n\nТишина.");
  } else {
    ShowStoryEvent(
        "Шеф пробует блюдо. Долгая пауза.\n\n"
        "«Сносно. На один раз. Можешь идти.»\n\n"
        "Он толкает ваш паспорт через стол.\n"
        "Вы свободны. Но слова «на один раз» будут сниться вам ещё долго.");
  }
}

static void RunFloor(GameState& state) {
  ShowFloorBanner(state.current_floor);

  if (state.current_floor == FLOOR_1 && !state.player.has_memory_item) {
    state.player.has_memory_item = true;
    ShowStoryEvent(
        "Шеф размахивает вашим паспортом.\n"
        "Вы смотрите на фото. Вы моложе. Жизнь ещё не сломала вас.\n"
        "Пухлые щёки — бабушка всегда кормила вас чем-то вкусным.\n\n"
        "Вы вспоминаете тот вкус. Первый компонент вашего последнего "
        "рецепта.\n\n"
        "«Воспоминания о забытых вкусах» — получено.");
  }

  HandleHall(state);
}

int main() {
#ifdef _WIN32
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
#endif

  const std::vector<std::string> kDataPaths = {"data", "../data", "../../data",
                                               "./PlateOfFate/data"};
  std::string data_path = "data";
  for (const std::string& path : kDataPaths) {
    std::ifstream test(path + "/config.csv");
    if (test.is_open()) {
      data_path = path;
      break;
    }
  }

  while (true) {
    ShowMainMenu();
    int choice = ReadInt(1, 2);
    if (choice == 2) {
      std::cout << "\nДо свидания.\n";
      return 0;
    }
    break;
  }

  GameState state = InitGame(data_path);

  if (state.floors.empty() || state.floors[0].customers.empty()) {
    std::cout << "\n[ОШИБКА] Не удалось загрузить данные игры.\n";
    std::cout
        << "Убедитесь, что папка 'data' с CSV-файлами находится рядом с exe.\n";
    PressEnter();
    return 1;
  }

  ShowStoryEvent(
      "Ресторан «Тарелка Судьбы». Утро.\n\n"
      "Вы написали заявление на увольнение. Годы без отдыха. Хватит.\n\n"
      "Шеф смотрит на листок. Берёт ваш паспорт. Усмехается.\n\n"
      "«Хочешь уйти? Пожалуйста. Но сначала — обслужи моих клиентов.\n"
      "Все три этажа. И придумай новое блюдо для меню.\n"
      "Тогда — свободен.»\n\n"
      "Выбора нет. Вы идёте на первый этаж.");

  while (state.is_running) {
    RunFloor(state);
    if (!state.is_running) break;

    Floor& current_floor =
        state.floors[static_cast<int>(state.current_floor) - 1];
    if (!CanProgressToNextFloor(current_floor) && CheckLossCondition(state)) {
      state.ending = ENDING_TRAPPED;
      state.is_running = false;
      break;
    }

    if (state.current_floor == FLOOR_1) {
      state.current_floor = FLOOR_2;
    } else if (state.current_floor == FLOOR_2) {
      state.current_floor = FLOOR_3;
    } else {
      HandleBossOffice(state);
      break;
    }
  }

  ShowEnding(state.ending);
  std::cout << "\nСпасибо за игру!\n\n";
  return 0;
}
