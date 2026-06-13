#include "csv_loader.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

static std::vector<std::string> SplitCsvLine(const std::string& line) {
  std::vector<std::string> result;
  std::string field;
  bool in_quotes = false;
  for (size_t i = 0; i < line.size(); ++i) {
    char symbol = line[i];
    if (symbol == '"') {
      in_quotes = !in_quotes;
    } else if (symbol == ',' && !in_quotes) {
      result.push_back(field);
      field.clear();
    } else {
      field += symbol;
    }
  }
  result.push_back(field);
  return result;
}

IngredientType IngredientFromString(const std::string& name) {
  if (name == "COFFEE_BEANS") return COFFEE_BEANS;
  if (name == "SUGAR") return SUGAR;
  if (name == "BREAD") return BREAD;
  if (name == "HAM") return HAM;
  if (name == "LETTUCE") return LETTUCE;
  if (name == "TOMATO") return TOMATO;
  if (name == "MAYO") return MAYO;
  if (name == "ONION") return ONION;
  if (name == "CHEESE") return CHEESE;
  if (name == "GARLIC") return GARLIC;
  if (name == "BEER") return BEER;
  if (name == "CHOCOLATE") return CHOCOLATE;
  if (name == "DOUGH") return DOUGH;
  if (name == "MEAT") return MEAT;
  if (name == "FISH") return FISH;
  if (name == "RICE") return RICE;
  if (name == "BUTTER") return BUTTER;
  if (name == "WINE") return WINE;
  if (name == "BEETROOT") return BEETROOT;
  return NONE;
}

std::string IngredientToString(IngredientType ingredient) {
  switch (ingredient) {
    case COFFEE_BEANS:
      return "Кофейные зёрна";
    case SUGAR:
      return "Сахар";
    case BREAD:
      return "Хлеб";
    case HAM:
      return "Ветчина";
    case LETTUCE:
      return "Салат";
    case TOMATO:
      return "Помидор";
    case MAYO:
      return "Майонез";
    case ONION:
      return "Лук";
    case CHEESE:
      return "Сыр";
    case GARLIC:
      return "Чеснок";
    case BEER:
      return "Пиво";
    case CHOCOLATE:
      return "Шоколад";
    case DOUGH:
      return "Тесто";
    case MEAT:
      return "Мясо";
    case FISH:
      return "Рыба";
    case RICE:
      return "Рис";
    case BUTTER:
      return "Сливочное масло";
    case WINE:
      return "Вино";
    case BEETROOT:
      return "Свекла";
    default:
      return "???";
  }
}

bool IsCookedDish(IngredientType token) {
  return static_cast<int>(token) >= static_cast<int>(INGREDIENT_COUNT);
}

int CookedDishRecipeId(IngredientType token) {
  return static_cast<int>(token) - static_cast<int>(INGREDIENT_COUNT);
}

std::string CookedDishName(IngredientType token,
                           const std::vector<Recipe>& recipes) {
  int recipe_id = CookedDishRecipeId(token);
  for (const Recipe& recipe : recipes) {
    if (recipe.id == recipe_id) {
      return recipe.name;
    }
  }
  return "Готовое блюдо";
}

std::string RecipeIngredientsList(const Recipe& recipe) {
  std::string result;
  for (size_t i = 0; i < recipe.ingredients.size(); ++i) {
    if (i > 0) result += " + ";
    result += IngredientToString(recipe.ingredients[i]);
  }
  return result;
}

GameConfig LoadConfig(const std::string& path) {
  GameConfig config = {};
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "[ERROR] Cannot open config: " << path << std::endl;
    config.floor1_total_customers = 10;
    config.floor1_min_to_progress = 6;
    config.floor1_patience = 3;
    config.floor2_total_groups = 3;
    config.floor2_min_to_progress = 2;
    config.floor2_patience = 4;
    config.floor2_group_size = 4;
    config.floor3_total_vip = 6;
    config.floor3_min_to_progress = 3;
    config.floor3_patience = 2;
    config.inventory_size = 5;
    config.upgrade_cost = 5;
    config.points_per_customer = 1;
    return config;
  }

  std::map<std::string, int> values;
  std::string line;
  std::getline(file, line);
  while (std::getline(file, line)) {
    if (line.empty()) continue;
    std::vector<std::string> parts = SplitCsvLine(line);
    if (parts.size() >= 2) {
      try {
        values[parts[0]] = std::stoi(parts[1]);
      } catch (...) {
      }
    }
  }

  auto Get = [&](const std::string& key, int fallback) {
    return values.count(key) ? values[key] : fallback;
  };

  config.floor1_total_customers = Get("floor1_total_customers", 10);
  config.floor1_min_to_progress = Get("floor1_min_to_progress", 6);
  config.floor1_patience = Get("floor1_patience", 3);
  config.floor2_total_groups = Get("floor2_total_groups", 3);
  config.floor2_min_to_progress = Get("floor2_min_to_progress", 2);
  config.floor2_patience = Get("floor2_patience", 4);
  config.floor2_group_size = Get("floor2_group_size", 4);
  config.floor3_total_vip = Get("floor3_total_vip", 6);
  config.floor3_min_to_progress = Get("floor3_min_to_progress", 3);
  config.floor3_patience = Get("floor3_patience", 2);
  config.inventory_size = Get("inventory_size", 5);
  config.upgrade_cost = Get("upgrade_cost", 5);
  config.points_per_customer = Get("points_per_customer", 1);
  return config;
}

std::vector<Recipe> LoadRecipes(const std::string& path) {
  std::vector<Recipe> recipes;
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "[ERROR] Cannot open recipes: " << path << std::endl;
    return recipes;
  }

  constexpr int kMinRecipeCsvFields = 7;
  std::string line;
  std::getline(file, line);
  while (std::getline(file, line)) {
    if (line.empty()) continue;
    std::vector<std::string> parts = SplitCsvLine(line);
    if (static_cast<int>(parts.size()) < kMinRecipeCsvFields) continue;

    Recipe recipe;
    recipe.id = std::stoi(parts[0]);
    recipe.name = parts[1];
    for (int i = 2; i <= 5; ++i) {
      if (!parts[i].empty()) {
        IngredientType ingredient = IngredientFromString(parts[i]);
        if (ingredient != NONE) recipe.ingredients.push_back(ingredient);
      }
    }
    recipe.floor_found = std::stoi(parts[6]);
    recipes.push_back(recipe);
  }
  return recipes;
}

std::vector<Customer> LoadCustomers(const std::string& path,
                                    const GameConfig& config) {
  std::vector<Customer> customers;
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "[ERROR] Cannot open customers: " << path << std::endl;
    return customers;
  }

  constexpr int kMinCustomerCsvFields = 7;
  std::string line;
  std::getline(file, line);
  while (std::getline(file, line)) {
    if (line.empty()) continue;
    std::vector<std::string> parts = SplitCsvLine(line);
    if (static_cast<int>(parts.size()) < kMinCustomerCsvFields) continue;

    Customer customer;
    customer.id = std::stoi(parts[0]);
    customer.floor = std::stoi(parts[1]);

    const std::string& type_str = parts[2];
    if (type_str == "single")
      customer.type = CUSTOMER_SINGLE;
    else if (type_str == "group")
      customer.type = CUSTOMER_GROUP;
    else
      customer.type = CUSTOMER_VIP;

    customer.description = parts[3];
    customer.hint = parts[4];
    customer.correct_recipe_id = std::stoi(parts[5]);
    customer.is_escape_trigger = (parts[6] == "1");
    customer.is_served = false;
    customer.has_left = false;

    if (customer.floor == 1)
      customer.patience = config.floor1_patience;
    else if (customer.floor == 2)
      customer.patience = config.floor2_patience;
    else
      customer.patience = config.floor3_patience;

    customers.push_back(customer);
  }
  return customers;
}
