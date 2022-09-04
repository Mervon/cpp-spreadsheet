#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
      [&](const auto& x) {
          output << x;
      },
      value);
    return output;
}
using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (pos.row < 0 || pos.row >= Position::MAX_ROWS || pos.col < 0 || pos.col >= Position::MAX_COLS) {
        throw InvalidPositionException("Invalid position");
    }

    if (auto cell = GetCell(pos); cell != nullptr) {
        dynamic_cast<Cell*>(cell)->Set(text);
    } else {

        sheet_[pos.row][pos.col] = std::make_unique<Cell>(*this);
        try {
            sheet_[pos.row][pos.col].get()->Set(text);
        } catch (CircularDependencyException& e) {
            if (sheet_[pos.row].size() == 1) {
                sheet_.erase(pos.row);
            } else {
                sheet_[pos.row].erase(pos.col);
            }

            throw CircularDependencyException(e.what());
        }
    }

    if (is_first_set_) {
        right_ = pos.col;
        left_ = right_;

        bottom_ = pos.row;
        top_ = bottom_;

        is_first_set_ = false;

        return;
    }

    if (pos.col > right_) {
        right_ = pos.col;
    }

    if (pos.col < left_) {
        left_ = pos.col;
    }

    if (pos.row > bottom_) {
        bottom_ = pos.row;
    }

    if (pos.row < top_) {
        top_ = pos.row;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.row < 0 || pos.row >= Position::MAX_ROWS || pos.col < 0 || pos.col >= Position::MAX_COLS) {
        throw InvalidPositionException("Invalid position");
    }

    if (auto pos_in_map_1 = sheet_.find(pos.row); pos_in_map_1 != sheet_.end()) {
        if (auto pos_in_map_2 = pos_in_map_1->second.find(pos.col); pos_in_map_2 != pos_in_map_1->second.end()) {
            return pos_in_map_2->second.get();
        }
    }

    return nullptr;

}
CellInterface* Sheet::GetCell(Position pos) {
    if (pos.row < 0 || pos.row >= Position::MAX_ROWS || pos.col < 0 || pos.col >= Position::MAX_COLS) {
        throw InvalidPositionException("Invalid position");
    }

    if (auto pos_in_map_1 = sheet_.find(pos.row); pos_in_map_1 != sheet_.end()) {
        if (auto pos_in_map_2 = pos_in_map_1->second.find(pos.col); pos_in_map_2 != pos_in_map_1->second.end()) {
            return pos_in_map_2->second.get();
        }
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (pos.row < 0 || pos.row >= Position::MAX_ROWS || pos.col < 0 || pos.col >= Position::MAX_COLS) {
        throw InvalidPositionException("Invalid position");
    }

    if (auto pos_in_map_1 = sheet_.find(pos.row); pos_in_map_1 != sheet_.end()) {
        if (auto pos_in_map_2 = pos_in_map_1->second.find(pos.col); pos_in_map_2 != pos_in_map_1->second.end()) {



            //std::cout << 555 << std::endl;
            pos_in_map_2->second->Clear();
            //pos_in_map_2->second->Set("");
            //std::cout << 555 << std::endl;

            if (pos_in_map_1->second.size() == 1) {
                sheet_.erase(pos_in_map_1);
            } else {
                pos_in_map_1->second.erase(pos_in_map_2);
            }

            if (pos.row == bottom_ || pos.row == top_ || pos.col == right_ || pos.col == left_) {
                RecalculateArea();
            }
        }
    }

}

void Sheet::RecalculateArea() {
    if (sheet_.size() == 0) {
        is_first_set_ = true;
        left_ = -1;
        right_ = -1;
        top_ = -1;
        bottom_ = -1;
        return;
    }

    auto first_item = sheet_.begin();

    top_ = first_item->first;
    bottom_ = top_;

    left_ = first_item->second.begin()->first;
    right_ = left_;

    for (const auto& item_1 : sheet_) {
        if (item_1.first > bottom_) {
            bottom_ = item_1.first;
        }

        if (item_1.first < top_) {
            top_ = item_1.first;
        }

        for (const auto& item_2 : item_1.second) {
            if (item_2.first > right_) {
                right_ = item_2.first;
            }

            if (item_2.first < left_) {
                left_ = item_2.first;
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    if (bottom_ == -1 || top_ == -1 || right_ == -1 || left_ == -1) {
        return Size{0, 0};
    }
    return Size{bottom_ - top_ + 1, right_ - left_ + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    if (sheet_.size() == 0) {
        return;
    }

    for (int i = top_; i <= bottom_; ++i) {
        for (int j = left_; j < right_; ++j) {
            if (const auto* position = GetCell(Position{i, j}); position != nullptr) {
                output << position->GetValue();
            }

            output << '\t';
        }

        if (const auto* position = GetCell(Position{i, right_}); position != nullptr) {
            output << position->GetValue();
        }

        output << '\n';
    }

}

void Sheet::PrintTexts(std::ostream& output) const {
    if (sheet_.size() == 0) {
        return;
    }

    for (int i = top_; i <= bottom_; ++i) {
        for (int j = left_; j < right_; ++j) {
            if (const auto* position = GetCell(Position{i, j}); position != nullptr) {
                output << position->GetText();
            }

            output << '\t';
        }

        if (const auto* position = GetCell(Position{i, right_}); position != nullptr) {
            output << position->GetText();
        }

        output << '\n';
    }

}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
