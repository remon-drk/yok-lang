#include <vector>
#include <string>
#include <stdexcept>

#include "token.hpp"
#include "instr.hpp"
#include "parser.hpp"
#include "field.hpp"
#include "error.hpp"

#define NO_TOKEN {}
#define ERR_MSG_UNDEF_INSTR "unknown instruction"
#define ERR_MSG_INCORRECT_PLURAL "plural suffix must be used correctly"

Parser::Parser(std::vector<TokenLine> &p_tokenlines)
    : m_tokenlines(p_tokenlines) {}

#define M Match
std::vector<Instr> Parser::get_instrs() {
    for (size_t i = 0; i < m_tokenlines.size(); i++) {
        m_curr_tokenline = &m_tokenlines[i];
        const size_t prev_instrs_len = m_instrs.size();

        try_parse_instr({
            M("put"),
            M("a"),
            M("sign"),
            M("here"),
            M("that"),
            M("reads"),
            M(MatchType::ID),
        }, InstrType::SIGN);

        try_parse_instr({
            M("hop"),
            M("to"),
            M("the"),
            M("sign"),
            M("that"),
            M("reads"),
            M(MatchType::ID),
        }, InstrType::HOP);

        try_parse_instr({
            M("hop"),
            M("to"),
            M("the"),
            M("sign"),
            M("below"),
            M("that"),
            M("reads"),
            M(MatchType::ID),
        }, InstrType::HOP_BELOW);

        try_parse_instr({
            M("hop"),
            M("to"),
            M("the"),
            M("sign"),
            M("above"),
            M("that"),
            M("reads"),
            M(MatchType::ID),
        }, InstrType::HOP_ABOVE);

        try_parse_instr({
            M("hop"),
            M("to"),
            M("the"),
            M("sign"),
            M("that"),
            M("reads"),
            M(MatchType::ID),
            M("but"),
            M("return"),
            M("back"),
            M("when"),
            M("you're"),
            M("done"),
        }, InstrType::HOP_BUT_RETUN);

        try_parse_instr({
            M("return"),
            M("back"),
            M("to"),
            M("the"),
            M("last"),
            M("place"),
            M("we"),
            M("said"),
            M("we'll")
        }, InstrType::RETURN);

        try_parse_instr({
            M("create"),
            M("a"),
            M("variable"),
            M("and"),
            M("call"),
            M("it"),
            M(MatchType::ID),
        }, InstrType::CREATE_VAR);

        try_parse_instr({
            M("assign"),
            M(MatchType::VALUE),
            M("to"),
            M(MatchType::ID),
        }, InstrType::ASSIGN_VAR);

        try_parse_instr({
            M("wait"),
            M("for"),
            M("user"),
            M("input"),
        }, InstrType::INPUT);

        try_parse_instr({
            M("say"),
            M(MatchType::VALUE),
            M("out"),
            M("loud"),
        }, InstrType::OUTPUT);

        try_parse_instr({
            M("whisper"),
            M(MatchType::VALUE),
            M("silently"),
        }, InstrType::OUTPUT_NO_NEWL);

        try_parse_instr({
            M("if"),
            M(MatchType::VALUE),
            M(MatchType::ID),
            M(MatchType::VALUE),
            M("skip"),
            M("next"),
            M(MatchType::QUANTITY),
            M("line"),
        }, InstrType::IF_SKIP);

        try_parse_instr({
            M("divide"),
            M(MatchType::VALUE),
            M("by"),
            M(MatchType::VALUE),
        }, InstrType::DIVIDE);

        try_parse_instr({
            M("multiply"),
            M(MatchType::VALUE),
            M("by"),
            M(MatchType::VALUE),
        }, InstrType::MULTIPLY);

        try_parse_instr({
            M("add"),
            M(MatchType::VALUE),
            M("and"),
            M(MatchType::VALUE),
        }, InstrType::ADD);

        try_parse_instr({
            M("subtract"),
            M(MatchType::VALUE),
            M("from"),
            M(MatchType::VALUE),
        }, InstrType::SUBTRACT);

        try_parse_instr({
            M("concatenate"),
            M(MatchType::VALUE),
            M("and"),
            M(MatchType::VALUE),
        }, InstrType::CONCAT);

        if (m_instrs.size() <= prev_instrs_len)
            throw std::invalid_argument(make_err_msg(
                m_curr_tokenline->line_no,
                m_curr_tokenline->tokens[0].col_no,
                ERR_MSG_UNDEF_INSTR));
    }

    return m_instrs;
}
#undef M

void Parser::try_parse_instr(const std::vector<Match> &p_pattern, InstrType type) {
    if (p_pattern.size() != m_curr_tokenline->tokens.size())
        return;

    Instr instr(type, m_curr_tokenline->line_no, m_curr_tokenline->tokens[0].col_no);
    bool must_end_with_s = false;

    for (size_t i = 0; i < p_pattern.size(); i++) {
        Token &tok = m_curr_tokenline->tokens[i];
        const Match &match = p_pattern[i];

        if (must_end_with_s) {
            if (tok.value.back() != 's')
                throw(make_err_msg(
                    m_curr_tokenline->line_no,
                    tok.col_no,
                    ERR_MSG_INCORRECT_PLURAL
                ));
            tok.value.pop_back();
        }

        if (match.type == MatchType::KEYWORD && tok.type == TokenType::WORD && tok.value == match.value)
            continue;

        if (match.type == MatchType::ID && tok.type == TokenType::WORD) {
            instr.args.push_back(Argument(Field(tok.value), ArgType::ID));
            continue;
        }

        if (match.type == MatchType::VALUE) {
            if (tok.type == TokenType::WORD) {
                instr.args.push_back(Argument(Field(tok.value), ArgType::ID));
                continue;
            }

            if (tok.type != TokenType::LIT_NUM && tok.type != TokenType::LIT_STR)
                return;

            Argument arg(Field(tok.value), ArgType::VAL);
            if (tok.type == TokenType::LIT_NUM)
                arg.value = Field(std::stof(tok.value));

            instr.args.push_back(arg);
            continue;
        }

        if (match.type == MatchType::QUANTITY && tok.type == TokenType::LIT_NUM) {
            if (std::stof(tok.value) != 1)
                must_end_with_s = true;
            instr.args.push_back(Argument(Field(std::stof(tok.value)), ArgType::VAL));
            continue;
        }

        return;
    }

    m_instrs.push_back(instr);
}