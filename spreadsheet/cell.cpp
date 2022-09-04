#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : sheet_(sheet) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    //std::cout << ":b4:" << parents_.size() << ":b4:" << std::endl;

    CheckForCircular(text);

    raw_value_ = text;

    //std::cout << 666 << std::endl;

    RecalculateParents();

    //std::cout << 666 << std::endl;

    CalculateValue();

    //std::cout << 666 << std::endl;

    RecalculateChilds();
    //std::cout << ":after:" << parents_.size() << ":after:" << std::endl;
    //std::cout << 666 << std::endl;
}

void Cell::CheckForCircular(std::string text) {

    std::vector<Position> temp_references;

    try {
        if (text.size() > 0) {
            std::unique_ptr<FormulaInterface> formula = ParseFormula(std::string(text.substr(1)));
            temp_references = formula.get()->GetReferencedCells();
        } else {
            throw std::runtime_error("temp");
        }
    } catch (...) {
        return;
    }

    std::vector<CellInterface*> pointers;

    for (const Position& pos : temp_references) {
        if (sheet_.GetCell(pos) != nullptr) {
            if (dynamic_cast<Cell*>(sheet_.GetCell(pos)) == this) {
                throw CircularDependencyException("Circular dependency found");
            }

            if (childs_.count(sheet_.GetCell(pos)) == 1) {
                throw CircularDependencyException("Circular dependency found");
            }


            pointers.push_back(sheet_.GetCell(pos));
        }
    }

    for (auto& child : childs_) {
        dynamic_cast<Cell*>(child)->CheckForCircularInternal(pointers);
    }
}

void Cell::CheckForCircularInternal(const std::vector<CellInterface*>& pointers) {
    for (CellInterface* pos : pointers) {
        if (childs_.count(pos) == 1) {
            throw CircularDependencyException("Circula dependency found");
        }
    }

    for (auto& child : childs_) {
        dynamic_cast<Cell*>(child)->CheckForCircularInternal(pointers);
    }
}

void Cell::DeleteFromChilds() {
    for (auto& child : childs_) {
        //std::cout << ":b4:" << reinterpret_cast<Cell*>(child)->parents_.size() << ":b4:" << std::endl;
        reinterpret_cast<Cell*>(child)->parents_.erase(reinterpret_cast<CellInterface*>(this));
        //std::cout << ":after:" << reinterpret_cast<Cell*>(child)->parents_.size() << ":after:" << std::endl;
    }

}

void Cell::DeleteFromParents() {
    for (auto& parent : parents_) {
        reinterpret_cast<Cell*>(parent)->childs_.erase(reinterpret_cast<CellInterface*>(this));
    }

    parents_.clear();

    references_.clear();
}

void Cell::RecalculateParents() {
    //std::cout << 333 << std::endl;
    //std::cout << ":" << parents_.size()  << ":" << std::endl;
    for (auto& parent : parents_) {
        reinterpret_cast<Cell*>(parent)->childs_.erase(reinterpret_cast<CellInterface*>(this));
    }

    //std::cout << 333 << std::endl;

    parents_.clear();

    references_.clear();

    try {
        if (raw_value_.size() > 0) {
            std::unique_ptr<FormulaInterface> formula = ParseFormula(std::string(raw_value_.substr(1)));
            references_ = formula.get()->GetReferencedCells();
        } else {
            throw std::runtime_error("temp");
        }
    } catch (...) {
        return;
    }

    for (auto& item : references_) {

        if (auto cell = sheet_.GetCell(item); cell != nullptr) {

            reinterpret_cast<Cell*>(cell)->GetChilds().insert(this);
            parents_.insert(cell);
        } else {
            sheet_.SetCell(item, "");
            auto cell_2 = sheet_.GetCell(item);
            reinterpret_cast<Cell*>(cell_2)->GetChilds().insert(this);
            parents_.insert(cell_2);
        }
    }
}

void Cell::RecalculateChilds() {
    for (auto& child : childs_) {
        dynamic_cast<Cell*>(child)->CalculateValue();
        dynamic_cast<Cell*>(child)->RecalculateChilds();
    }
}

void Cell::CalculateValue() {
    std::string text = raw_value_;

    if (text.size() == 0) {
        value_ = std::make_unique<Value>("");
        return;
    }

    if (text.size() == 1) {
        if (text[0] == ESCAPE_SIGN) {
            value_ = std::make_unique<Value>("");
        } else if (text[0] == FORMULA_SIGN) {
            value_ = std::make_unique<Value>("=");
        } else {
            value_ = std::make_unique<Value>(text);
        }

        return;
    }

    if (text.size() > 0) {
        if (text[0] == ESCAPE_SIGN) {
            text = text.substr(1);
            value_ = std::make_unique<Value>(text);
        } else if (text[0] == FORMULA_SIGN) {
            is_formula_ = true;
            text = text.substr(1);

            FormulaInterface::Value formula_2 = ParseFormula(text).get()->Evaluate(sheet_);
            if (std::holds_alternative<double>(formula_2)) {
                value_ = std::make_unique<CellInterface::Value>(std::get<double>(formula_2));
            } else if (std::holds_alternative<FormulaError>(formula_2)) {
                value_ = std::make_unique<CellInterface::Value>(std::get<FormulaError>(formula_2));
            }
        } else {
            value_ = std::make_unique<Value>(text);
        }
    }
}

void Cell::Clear() {
    raw_value_ = "";
    value_ = std::make_unique<Value>("");
    RecalculateChilds();
    DeleteFromChilds();
    DeleteFromParents();
    childs_.clear();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return references_;
}

CellInterface::Value Cell::GetValue() const {

    CellInterface::Value val = *(value_.get());

    return val;
}
std::string Cell::GetText() const {
    if (is_formula_) {
        std::string formula = "=";
        formula += ParseFormula(std::string(raw_value_.substr(1))).get()->GetExpression();
        return formula;
    } else {
        return raw_value_;
    }

}
