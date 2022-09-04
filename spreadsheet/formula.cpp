#include "formula.h"

#include "FormulaAST.h"

#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {

class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression);
    FormulaInterface::Value Evaluate(const SheetInterface& sheet) const override;
    std::string GetExpression() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    std::unique_ptr<FormulaAST> ast_;
};

Formula::Formula(std::string expression)  {
    try {
        ast_ = std::unique_ptr<FormulaAST>(new FormulaAST(ParseFormulaAST(std::move(expression))));
    } catch (...) {
        throw FormulaException("Error while parsing formula");
    }
}

FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const {
    try {
        return ast_->Execute(sheet);
    } catch (FormulaError& fe) {
        switch(fe.GetCategory()) {
            case FormulaError::Category::Div0: {
                return FormulaError{FormulaError::Category::Div0};
                break;
            }
            case FormulaError::Category::Value: {
                return FormulaError{FormulaError::Category::Value};
                break;
            }
            case FormulaError::Category::Ref: {
                return FormulaError{FormulaError::Category::Ref};
                break;
            }
            default: {
                break;
            }
        }
        
        return -999.0;
    }
}

std::vector<Position> Formula::GetReferencedCells() const {
    std::set<Position> positions(ast_->GetCells().begin(), ast_->GetCells().end());
    return std::vector(positions.begin(), positions.end());
}

std::string Formula::GetExpression() const {
    std::stringstream ss;
    ast_->PrintFormula(ss);
    return ss.str();
}
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}

