#pragma once

#include "cell.h"
#include "common.h"

#include <unordered_map>
#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    void RecalculateArea();

	std::unordered_map<int, std::unordered_map<int, std::unique_ptr<Cell>>> sheet_;
	bool is_first_set_ = true;
	int left_ = -1;
	int right_ = -1;
	int top_ = -1;
	int bottom_ = -1;
};
