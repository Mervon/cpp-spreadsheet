#pragma once

#include "common.h"
#include "formula.h"

#include <string>
#include <set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    std::set<CellInterface*>& GetChilds() {
        return childs_;
    }

    std::set<CellInterface*>& GetParents() {
        return parents_;
    }

    bool IsReferenced() const;

private:

    void CheckForCircular(std::string text);

    void CheckForCircularInternal(const std::vector<CellInterface*>& pointers);

    void RecalculateChilds();

    void CalculateValue();

    void RecalculateParents();

    void DeleteFromParents();

    void DeleteFromChilds();

    std::set<CellInterface*> parents_;
    std::set<CellInterface*> childs_;
    SheetInterface& sheet_;
    std::unique_ptr<CellInterface::Value> value_;
    std::string raw_value_;

    std::vector<Position> references_;
    bool is_formula_ = false;
    bool is_empty_cell = true;
};
