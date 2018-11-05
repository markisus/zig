#include <iostream>
#include "zig.h"

using namespace zig;
int main(int argc, char** argv) {
  // Automobile shop example from Ch.2 of 
  // Applied Mathematical Programming 
  // by Bradley, Hax, and Magnanti
  Tableau tableau;
  tableau.objective = 0;

  tableau.rhs_values = {24, 60};
  tableau.basic_variable_ids = {4, 5};

  // x1
  {
    Column column;
    column.variable_id = 1;
    column.objective_coefficient = 6;
    column.coefficients.push_back(
        Coefficient(0, 0.5));
    column.coefficients.push_back(
        Coefficient(1, 1.0));
    tableau.nonbasic_variables.push_back(
        std::move(column));
  }

  // x2
  {
    Column column;
    column.variable_id = 2;
    column.objective_coefficient = 14;
    column.coefficients.push_back(
        Coefficient(0, 2));
    column.coefficients.push_back(
        Coefficient(1, 2));
    tableau.nonbasic_variables.push_back(
        std::move(column));
  }

  // x3
  {
    Column column;
    column.variable_id = 3;
    column.objective_coefficient = 13;
    column.coefficients.push_back(
        Coefficient(0, 1));
    column.coefficients.push_back(
        Coefficient(1, 4));
    tableau.nonbasic_variables.push_back(
        std::move(column));
  }
  
  Maximize(&tableau);

  std::cout << "Maximized to " << tableau.objective << std::endl;
}
