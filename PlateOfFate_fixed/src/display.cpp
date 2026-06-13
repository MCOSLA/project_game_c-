#include "display.h"

#include <iomanip>
#include <iostream>
#include <string>

#include "csv_loader.h"

#ifdef _WIN32
#include <windows.h>
#endif

void ClearScreen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void PrintSeparator() {
  std::cout << "=================================================\n";
}

void PrintTitle(const std::string& title) {
  constexpr int kSeparatorWidth = 44;
  PrintSeparator();
  int pad = (kSeparatorWidth - static_cast<int>(title.size())) / 2;
  if (pad < 0) pad = 0;
  std::cout << std::string(pad, ' ') << title << "\n";
  PrintSeparator();
}

void PressEnter() {
  std::cout << "\n[Нажмите Enter для продолжения...]";
  std::cin.ignore(1000, '\n');
  std::cin.get();
}

void ShowMainMenu() {
  ClearScreen();
  PrintSeparator();
  std::cout << "\n";
  std::cout << "████╗░████╗░████╗░████╗░████╗░████╗░████╗░████╗░\n";
  std::cout << "                 PLATE OF FATE\n";
  std::cout << "\n";
  PrintSeparator();
  std::cout << "  Сможете ли вы вернуть себе свободу?\n";
  std::cout
      << "  Вы - простой повар, которому уже пора найти выход на волю.\n\n";
  std::cout << "  1. Начать игру\n";
  std::cout << "  2. Выход\n\n";
  std::cout << "Ваш выбор: ";
}

void ShowFloorBanner(FloorLevel floor) {
  ClearScreen();
  PrintSeparator();
  if (floor == FLOOR_1) {
    std::cout << "           ЭТАЖ 1 — ОБЫЧНЫЕ КЛИЕНТЫ\n";
    std::cout << "   Офисные работники. Спешат.\n";
  } else if (floor == FLOOR_2) {
    std::cout << "           ЭТАЖ 2 — ГРУППОВЫЕ ЗАКАЗЫ\n";
    std::cout << "   Семьи и компании.\n";
  } else {
    std::cout << "           ЭТАЖ 3 — VIP ЗАЛ\n";
    std::cout << "   Богатые гости. Капризны.\n";
  }
  PrintSeparator();
  PressEnter();
}

void ShowInventory(const Player& player, const std::vector<Recipe>& recipes) {
  std::cout << "\n--- Инвентарь [" << player.inventory.size() << "/"
            << player.max_inventory << "] ---\n";
  if (player.inventory.empty()) {
    std::cout << "  (пусто)\n";
  } else {
    for (size_t i = 0; i < player.inventory.size(); ++i) {
      IngredientType item = player.inventory[i];
      std::cout << "  " << (i + 1) << ". ";
      if (IsCookedDish(item)) {
        std::cout << "[БЛЮДО] " << CookedDishName(item, recipes);
      } else {
        std::cout << IngredientToString(item);
      }
      std::cout << "\n";
    }
  }
  if (player.has_key) std::cout << "  [КЛЮЧ от чёрного хода]\n";
  if (player.has_memory_item)
    std::cout << "  [Воспоминания о забытых вкусах]\n";
  if (player.has_truffle_item)
    std::cout << "  [Загадочный ингредиент — трюфель]\n";
  if (player.has_spice_item) std::cout << "  [Изюминка с перчинкой]\n";
  std::cout << "  Очков прокачки: " << player.upgrade_points << "\n";
}

void ShowStorageMenu(const Floor& floor, const Player& player,
                     const std::vector<Recipe>& recipes) {
  ClearScreen();
  PrintTitle("СКЛАД");
  ShowInventory(player, recipes);
  std::cout << "\n--- Доступные ингредиенты ---\n";
  for (size_t i = 0; i < floor.storage_items.size(); ++i) {
    std::cout << "  " << (i + 1) << ". "
              << IngredientToString(floor.storage_items[i]) << "\n";
  }
  std::cout << "\n";
}

void ShowKitchenMenu(const Floor& floor, const Player& player,
                     const std::vector<Recipe>& recipes) {
  ClearScreen();
  PrintTitle("КУХНЯ");
  ShowInventory(player, recipes);
  std::cout << "\n--- Доступные рецепты ---\n";
  for (size_t i = 0; i < floor.available_recipes.size(); ++i) {
    const Recipe& recipe = floor.available_recipes[i];
    std::cout << "  " << (i + 1) << ". " << recipe.name << " ["
              << RecipeIngredientsList(recipe) << "]\n";
  }
  std::cout << "\n";
}

void ShowCustomerList(const Floor& floor) {
  std::cout << "\n--- Клиенты на этаже ---\n";
  int index = 1;
  for (const Customer& customer : floor.customers) {
    if (customer.is_served || customer.has_left) continue;
    std::cout << "  " << index << ". " << customer.description;
    if (customer.type == CUSTOMER_GROUP) std::cout << " [ГРУППА]";
    if (customer.type == CUSTOMER_VIP) std::cout << " [VIP]";
    std::cout << "\n";
    ++index;
  }
  if (index == 1) std::cout << "  (нет активных клиентов)\n";
  std::cout << "\n";
}

void ShowCustomerDialog(const Customer& customer) {
  PrintSeparator();
  std::cout << "КЛИЕНТ: " << customer.description << "\n";
  PrintSeparator();
  std::cout << "\nКлиент говорит:\n\"" << customer.hint << "\"\n";
  std::cout << "\nТерпение: ";
  for (int i = 0; i < customer.patience; ++i) std::cout << "♥ ";
  std::cout << "\n\n";
}

void ShowRecipeChoice(const std::vector<Recipe>& recipes) {
  std::cout << "--- Выберите блюдо ---\n";
  for (size_t i = 0; i < recipes.size(); ++i) {
    std::cout << "  " << (i + 1) << ". " << recipes[i].name << " ["
              << RecipeIngredientsList(recipes[i]) << "]\n";
  }
  std::cout << "  0. Не предлагать блюдо (пропустить клиента)\n";
  std::cout << "\nВаш выбор: ";
}

void ShowRecipeBook(const std::vector<Recipe>& recipes) {
  ClearScreen();
  PrintTitle("КНИГА РЕЦЕПТОВ");
  for (const Recipe& recipe : recipes) {
    std::cout << "  " << recipe.name << "\n";
    std::cout << "    Ингредиенты: " << RecipeIngredientsList(recipe) << "\n\n";
  }
}

void ShowStoryEvent(const std::string& text) {
  ClearScreen();
  PrintSeparator();
  std::cout << "  СЮЖЕТ \n";
  PrintSeparator();
  std::cout << "\n" << text << "\n";
  PressEnter();
}

void ShowEnding(Ending ending) {
  ClearScreen();
  PrintSeparator();
  if (ending == ENDING_HONEST) {
    std::cout << "          КОНЦОВКА: «ЧЕСТНЫЙ ПОВАР»\n";
    PrintSeparator();
    std::cout << "\nШеф привередливо смотрит на блюдо и пробует.\n";
    std::cout << "Скрывая восхищение, он кивает:\n";
    std::cout << "«Сносный обед. На один раз. Можешь идти.»\n\n";
    std::cout << "Ты получил свободу. Но слова шефа\n";
    std::cout << "будут преследовать тебя долгие годы...\n\n";
    std::cout << " ПОБЕДА — нейтральная концовка \n";
  } else if (ending == ENDING_DECISION) {
    std::cout << "       КОНЦОВКА: «САМОСТОЯТЕЛЬНОЕ РЕШЕНИЕ»\n";
    PrintSeparator();
    std::cout << "\nТы накормил всех. Каждого. До последнего.\n";
    std::cout << "Ярость переполняет тебя.\n";
    std::cout << "Войдя в офис, ты не ставишь тарелку — ты бросаешь её.\n\n";
    std::cout << "Шеф молчит. Твои гениальные рецепты с тобой.\n";
    std::cout << "Ты открываешь свой ресторан.\n\n";
    std::cout << " ПОБЕДА — лучшая концовка \n";
  } else if (ending == ENDING_ESCAPE) {
    std::cout << "           КОНЦОВКА: «ПОБЕГ»\n";
    PrintSeparator();
    std::cout << "\nЧёрный ход открылся. Ты выходишь в ночь.\n";
    std::cout << "Без зарплаты. Без паспорта. Но свободен.\n\n";
    std::cout << "Когда-нибудь ты вернёшься — уже как гость,\n";
    std::cout << "чтобы передать ключ следующему.\n\n";
    std::cout << " ПОБЕДА — легкая концовка \n";
  } else if (ending == ENDING_TRAPPED) {
    std::cout << "        КОНЦОВКА: «В СТЕНАХ РЕСТОРАНА»\n";
    PrintSeparator();
    std::cout << "\nШеф со зловещей ухмылкой смотрит на тебя.\n";
    std::cout << "Он берёт твоё заявление... и разрывает его.\n\n";
    std::cout << "«Ты никуда не уйдёшь.»\n\n";
    std::cout << "До конца своих дней ты будешь работать здесь.\n\n";
    std::cout << " ПОРАЖЕНИЕ \n";
  }
  PrintSeparator();
  PressEnter();
}

void ShowUpgradeMenu(std::vector<Recipe>& recipes) {
  ClearScreen();
  PrintTitle("ПРОКАЧКА РЕЦЕПТА");
  std::cout << "Выберите рецепт для улучшения (убрать 1 ингредиент):\n\n";
  for (size_t i = 0; i < recipes.size(); ++i) {
    const Recipe& recipe = recipes[i];
    if (recipe.ingredients.size() >= 2) {
      std::cout << "  " << (i + 1) << ". " << recipe.name << " ["
                << RecipeIngredientsList(recipe) << "]\n";
    }
  }
  std::cout << "  0. Пропустить прокачку\n\n";
  std::cout << "Ваш выбор: ";
}
