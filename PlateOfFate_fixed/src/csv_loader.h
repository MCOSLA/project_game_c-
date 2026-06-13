#pragma once
#include <string>
#include <vector>

#include "types.h"

GameConfig LoadConfig(const std::string& path);

std::vector<Recipe> LoadRecipes(const std::string& path);

std::vector<Customer> LoadCustomers(const std::string& path,
                                    const GameConfig& config);

IngredientType IngredientFromString(const std::string& name);

std::string IngredientToString(IngredientType ingredient);

std::string CookedDishName(IngredientType token,
                           const std::vector<Recipe>& recipes);

bool IsCookedDish(IngredientType token);

int CookedDishRecipeId(IngredientType token);

std::string RecipeIngredientsList(const Recipe& recipe);
