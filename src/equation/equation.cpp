#include "uranium/equation/equation.hpp"
#include "uranium/core/error.hpp"
#include "uranium/render/font.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <unordered_map>
#include <vector>
namespace uranium {
namespace equation {
namespace {
using render::Color;
using render::Font;
using render::TextAlign;
using render::Vec2;
enum Style { StyleItalic = 0, StyleUpright = 1, StyleBold = 2 };
struct Node {
    enum class Kind { Atom, Row, Frac, Root, Sup, Sub, Accent } kind = Kind::Atom;
    std::string text;
    float scale = 1.0f;
    int style = StyleUpright;
    float space_before = 0.0f;
    float space_after = 0.0f;
    std::string accent_type;
    std::vector<Node> children;
};

void append_utf8(std::string& out, int cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

struct SymbolEntry {
    int cp;
    int style;
    float space_before;
    float space_after;
};

const std::unordered_map<std::string, SymbolEntry>& symbol_table() {
    static const std::unordered_map<std::string, SymbolEntry> table = {
        {"alpha", {0x3B1, StyleItalic, 0, 0}},
        {"beta", {0x3B2, StyleItalic, 0, 0}},
        {"gamma", {0x3B3, StyleItalic, 0, 0}},
        {"delta", {0x3B4, StyleItalic, 0, 0}},
        {"epsilon", {0x3B5, StyleItalic, 0, 0}},
        {"varepsilon", {0x3F5, StyleItalic, 0, 0}},
        {"zeta", {0x3B6, StyleItalic, 0, 0}},
        {"eta", {0x3B7, StyleItalic, 0, 0}},
        {"theta", {0x3B8, StyleItalic, 0, 0}},
        {"vartheta", {0x3D1, StyleItalic, 0, 0}},
        {"iota", {0x3B9, StyleItalic, 0, 0}},
        {"kappa", {0x3BA, StyleItalic, 0, 0}},
        {"lambda", {0x3BB, StyleItalic, 0, 0}},
        {"mu", {0x3BC, StyleItalic, 0, 0}},
        {"nu", {0x3BD, StyleItalic, 0, 0}},
        {"xi", {0x3BE, StyleItalic, 0, 0}},
        {"omicron", {0x3BF, StyleItalic, 0, 0}},
        {"pi", {0x3C0, StyleItalic, 0, 0}},
        {"varpi", {0x3D6, StyleItalic, 0, 0}},
        {"rho", {0x3C1, StyleItalic, 0, 0}},
        {"sigma", {0x3C3, StyleItalic, 0, 0}},
        {"varsigma", {0x3C2, StyleItalic, 0, 0}},
        {"tau", {0x3C4, StyleItalic, 0, 0}},
        {"upsilon", {0x3C5, StyleItalic, 0, 0}},
        {"phi", {0x3C6, StyleItalic, 0, 0}},
        {"varphi", {0x3D5, StyleItalic, 0, 0}},
        {"chi", {0x3C7, StyleItalic, 0, 0}},
        {"psi", {0x3C8, StyleItalic, 0, 0}},
        {"omega", {0x3C9, StyleItalic, 0, 0}},
        {"Gamma", {0x393, StyleUpright, 0, 0}},
        {"Delta", {0x394, StyleUpright, 0, 0}},
        {"Theta", {0x398, StyleUpright, 0, 0}},
        {"Lambda", {0x39B, StyleUpright, 0, 0}},
        {"Xi", {0x39E, StyleUpright, 0, 0}},
        {"Pi", {0x3A0, StyleUpright, 0, 0}},
        {"Sigma", {0x3A3, StyleUpright, 0, 0}},
        {"Upsilon", {0x3A5, StyleUpright, 0, 0}},
        {"Phi", {0x3A6, StyleUpright, 0, 0}},
        {"Psi", {0x3A8, StyleUpright, 0, 0}},
        {"Omega", {0x3A9, StyleUpright, 0, 0}},
        {"infty", {0x221E, StyleUpright, 0, 0}},
        {"partial", {0x2202, StyleItalic, 0, 0}},
        {"nabla", {0x2207, StyleUpright, 0, 0}},
        {"pm", {0x00B1, StyleUpright, 0.22f, 0.22f}},
        {"mp", {0x2213, StyleUpright, 0.22f, 0.22f}},
        {"times", {0x00D7, StyleUpright, 0.22f, 0.22f}},
        {"div", {0x00F7, StyleUpright, 0.22f, 0.22f}},
        {"cdot", {0x22C5, StyleUpright, 0.22f, 0.22f}},
        {"ast", {0x2217, StyleUpright, 0.22f, 0.22f}},
        {"circ", {0x2218, StyleUpright, 0.22f, 0.22f}},
        {"bullet", {0x2022, StyleUpright, 0.22f, 0.22f}},
        {"leq", {0x2264, StyleUpright, 0.28f, 0.28f}},
        {"le", {0x2264, StyleUpright, 0.28f, 0.28f}},
        {"geq", {0x2265, StyleUpright, 0.28f, 0.28f}},
        {"ge", {0x2265, StyleUpright, 0.28f, 0.28f}},
        {"neq", {0x2260, StyleUpright, 0.28f, 0.28f}},
        {"ne", {0x2260, StyleUpright, 0.28f, 0.28f}},
        {"approx", {0x2248, StyleUpright, 0.28f, 0.28f}},
        {"equiv", {0x2261, StyleUpright, 0.28f, 0.28f}},
        {"sim", {0x223C, StyleUpright, 0.28f, 0.28f}},
        {"simeq", {0x2243, StyleUpright, 0.28f, 0.28f}},
        {"cong", {0x2245, StyleUpright, 0.28f, 0.28f}},
        {"propto", {0x221D, StyleUpright, 0.28f, 0.28f}},
        {"ll", {0x226A, StyleUpright, 0.28f, 0.28f}},
        {"gg", {0x226B, StyleUpright, 0.28f, 0.28f}},
        {"to", {0x2192, StyleUpright, 0.28f, 0.28f}},
        {"rightarrow", {0x2192, StyleUpright, 0.28f, 0.28f}},
        {"leftarrow", {0x2190, StyleUpright, 0.28f, 0.28f}},
        {"leftrightarrow", {0x2194, StyleUpright, 0.28f, 0.28f}},
        {"Rightarrow", {0x21D2, StyleUpright, 0.28f, 0.28f}},
        {"Leftarrow", {0x21D0, StyleUpright, 0.28f, 0.28f}},
        {"Leftrightarrow", {0x21D4, StyleUpright, 0.28f, 0.28f}},
        {"mapsto", {0x21A6, StyleUpright, 0.28f, 0.28f}},
        {"uparrow", {0x2191, StyleUpright, 0.28f, 0.28f}},
        {"downarrow", {0x2193, StyleUpright, 0.28f, 0.28f}},
        {"sum", {0x2211, StyleUpright, 0.15f, 0.15f}},
        {"prod", {0x220F, StyleUpright, 0.15f, 0.15f}},
        {"coprod", {0x2210, StyleUpright, 0.15f, 0.15f}},
        {"int", {0x222B, StyleUpright, 0.1f, 0.1f}},
        {"oint", {0x222E, StyleUpright, 0.1f, 0.1f}},
        {"iint", {0x222C, StyleUpright, 0.1f, 0.1f}},
        {"ldots", {0x2026, StyleUpright, 0, 0}},
        {"cdots", {0x22EF, StyleUpright, 0, 0}},
        {"vdots", {0x22EE, StyleUpright, 0, 0}},
        {"ddots", {0x22F1, StyleUpright, 0, 0}},
        {"dots", {0x2026, StyleUpright, 0, 0}},
        {"cup", {0x222A, StyleUpright, 0.22f, 0.22f}},
        {"cap", {0x2229, StyleUpright, 0.22f, 0.22f}},
        {"setminus", {0x2216, StyleUpright, 0.22f, 0.22f}},
        {"in", {0x2208, StyleUpright, 0.28f, 0.28f}},
        {"notin", {0x2209, StyleUpright, 0.28f, 0.28f}},
        {"ni", {0x220B, StyleUpright, 0.28f, 0.28f}},
        {"subset", {0x2282, StyleUpright, 0.28f, 0.28f}},
        {"supset", {0x2283, StyleUpright, 0.28f, 0.28f}},
        {"subseteq", {0x2286, StyleUpright, 0.28f, 0.28f}},
        {"supseteq", {0x2287, StyleUpright, 0.28f, 0.28f}},
        {"forall", {0x2200, StyleUpright, 0, 0}},
        {"exists", {0x2203, StyleUpright, 0, 0}},
        {"nexists", {0x2204, StyleUpright, 0, 0}},
        {"emptyset", {0x2205, StyleUpright, 0, 0}},
        {"varnothing", {0x2205, StyleUpright, 0, 0}},
        {"angle", {0x2220, StyleUpright, 0, 0}},
        {"measuredangle", {0x2222, StyleUpright, 0, 0}},
        {"degree", {0x00B0, StyleUpright, 0, 0}},
        {"prime", {0x2032, StyleUpright, 0, 0}},
        {"hbar", {0x210F, StyleItalic, 0, 0}},
        {"ell", {0x2113, StyleItalic, 0, 0}},
        {"Re", {0x211C, StyleUpright, 0, 0}},
        {"Im", {0x2111, StyleUpright, 0, 0}},
        {"aleph", {0x2135, StyleUpright, 0, 0}},
        {"wp", {0x2118, StyleUpright, 0, 0}},
        {"top", {0x22A4, StyleUpright, 0, 0}},
        {"bot", {0x22A5, StyleUpright, 0, 0}},
        {"wedge", {0x2227, StyleUpright, 0.22f, 0.22f}},
        {"vee", {0x2228, StyleUpright, 0.22f, 0.22f}},
        {"land", {0x2227, StyleUpright, 0.22f, 0.22f}},
        {"lor", {0x2228, StyleUpright, 0.22f, 0.22f}},
        {"neg", {0x00AC, StyleUpright, 0, 0}},
        {"lnot", {0x00AC, StyleUpright, 0, 0}},
        {"mid", {0x2223, StyleUpright, 0.28f, 0.28f}},
        {"perp", {0x22A5, StyleUpright, 0.28f, 0.28f}},
        {"parallel", {0x2225, StyleUpright, 0.28f, 0.28f}},
        {"because", {0x2235, StyleUpright, 0.28f, 0.28f}},
        {"therefore", {0x2234, StyleUpright, 0.28f, 0.28f}},
        {"langle", {0x27E8, StyleUpright, 0, 0}},
        {"rangle", {0x27E9, StyleUpright, 0, 0}},
        {"lceil", {0x2308, StyleUpright, 0, 0}},
        {"rceil", {0x2309, StyleUpright, 0, 0}},
        {"lfloor", {0x230A, StyleUpright, 0, 0}},
        {"rfloor", {0x230B, StyleUpright, 0, 0}},
        {"backslash", {0x005C, StyleUpright, 0, 0}},
        {"{", {0x007B, StyleUpright, 0, 0}},
        {"}", {0x007D, StyleUpright, 0, 0}},
        {"%", {0x0025, StyleUpright, 0, 0}},
        {"&", {0x0026, StyleUpright, 0, 0}},
        {"#", {0x0023, StyleUpright, 0, 0}},
        {"_", {0x005F, StyleUpright, 0, 0}},
    };
    return table;
}

const std::unordered_map<std::string, float>& function_table() {
    static const std::unordered_map<std::string, float> table = {
        {"sin", 0.16f},   {"cos", 0.16f},   {"tan", 0.16f},   {"cot", 0.16f},
        {"sec", 0.16f},   {"csc", 0.16f},   {"arcsin", 0.16f}, {"arccos", 0.16f},
        {"arctan", 0.16f}, {"sinh", 0.16f}, {"cosh", 0.16f},  {"tanh", 0.16f},
        {"coth", 0.16f},  {"exp", 0.16f},   {"log", 0.16f},   {"ln", 0.16f},
        {"lg", 0.16f},    {"det", 0.16f},   {"dim", 0.16f},   {"gcd", 0.16f},
        {"deg", 0.16f},   {"arg", 0.16f},   {"ker", 0.16f},   {"hom", 0.16f},
        {"Pr", 0.16f},    {"tr", 0.16f},    {"traces", 0.16f},
    };
    return table;
}

bool is_function_name(const std::string& s) { return function_table().count(s) > 0; }
struct Parser {
    const std::string& src;
    std::size_t i = 0;
    bool failed = false;
    std::string error;
    explicit Parser(const std::string& s) : src(s) {}
    void fail(const std::string& msg) {
        if (!failed) {
            failed = true;
            error = msg;
        }
    }

    void skip_spaces() {
        while (i < src.size() && (src[i] == ' ' || src[i] == '\t' || src[i] == '\n' || src[i] == '\r')) ++i;
    }

    char peek() {
        skip_spaces();
        return i < src.size() ? src[i] : '\0';
    }

    std::string read_command_name() {
        ++i;
        if (i >= src.size()) {
            fail("trailing backslash");
            return std::string();
        }
        if (static_cast<unsigned char>(src[i]) < 0x80 && !((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z'))) {
            return std::string(1, src[i++]);
        }
        std::string name;
        while (i < src.size() && ((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z'))) name.push_back(src[i++]);
        return name;
    }

    Node make_atom(const std::string& text, int style, float sb = 0.0f, float sa = 0.0f, float scale = 1.0f) {
        Node n;
        n.kind = Node::Kind::Atom;
        n.text = text;
        n.style = style;
        n.space_before = sb;
        n.space_after = sa;
        n.scale = scale;
        return n;
    }

    Node parse_group() {
        char c = peek();
        if (c == '}' || c == '\0') {
            fail("missing brace group argument");
            return make_atom("", StyleUpright);
        }
        if (c == '{') {
            ++i;
            Node row = parse_row(false);
            if (peek() == '}') ++i;
            else fail("expected closing brace");
            return row;
        }
        return parse_single();
    }

    std::string read_raw_group() {
        skip_spaces();
        if (i >= src.size() || src[i] != '{') {
            fail("expected brace group");
            return std::string();
        }
        ++i;
        int depth = 1;
        std::string out;
        while (i < src.size()) {
            char c = src[i];
            if (c == '\\' && i + 1 < src.size()) {
                out.push_back(c);
                out.push_back(src[i + 1]);
                i += 2;
                continue;
            }
            if (c == '{') ++depth;
            if (c == '}') {
                --depth;
                if (depth == 0) {
                    ++i;
                    return out;
                }
            }
            out.push_back(c);
            ++i;
        }
        fail("unterminated brace group");
        return out;
    }

    Node parse_single() {
        char c = peek();
        if (c == '\0') return make_atom("", StyleUpright);
        if (c == '{') return parse_group();
        if (c == '\\') return parse_command();
        if (c == '(' || c == ')' || c == '[' || c == ']') {
            ++i;
            return make_atom(std::string(1, c), StyleUpright);
        }
        if (c >= '0' && c <= '9') {
            std::string run;
            while (i < src.size() && ((src[i] >= '0' && src[i] <= '9') || src[i] == '.')) run.push_back(src[i++]);
            return make_atom(run, StyleUpright);
        }
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            std::string run;
            while (i < src.size() && ((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z'))) run.push_back(src[i++]);
            return make_atom(run, StyleItalic);
        }
        ++i;
        if (c == '\'') return make_atom("\xE2\x80\xB2", StyleUpright, 0.0f, 0.02f);
        if (c == '+') return make_atom("+", StyleUpright, 0.22f, 0.22f);
        if (c == '-') return make_atom("\xE2\x88\x92", StyleUpright, 0.22f, 0.22f);
        if (c == '*' || c == '.') return make_atom(std::string(1, c), StyleUpright, 0.22f, 0.22f);
        if (c == '=' || c == '<' || c == '>') return make_atom(std::string(1, c), StyleUpright, 0.28f, 0.28f);
        if (c == ',' || c == ';' || c == ':') return make_atom(std::string(1, c), StyleUpright, 0.0f, 0.08f);
        if (c == '~') return make_atom("", StyleUpright, 0.17f, 0.0f);
        return make_atom(std::string(1, c), StyleUpright);
    }

    Node parse_command() {
        std::string name = read_command_name();
        if (failed) return make_atom("", StyleUpright);
        if (name == "frac" || name == "dfrac" || name == "tfrac" || name == "cfrac") {
            Node n;
            n.kind = Node::Kind::Frac;
            n.children.push_back(parse_group());
            n.children.push_back(parse_group());
            return n;
        }
        if (name == "sqrt") {
            Node n;
            n.kind = Node::Kind::Root;
            if (peek() == '[') {
                ++i;
                Node deg;
                deg.kind = Node::Kind::Row;
                while (i < src.size() && src[i] != ']') {
                    if (src[i] == '\\') deg.children.push_back(parse_command());
                    else deg.children.push_back(make_atom(std::string(1, src[i++]), StyleUpright));
                }
                if (i < src.size() && src[i] == ']') ++i;
                n.children.push_back(deg);
            } else {
                n.children.push_back(Node());
            }
            n.children.push_back(parse_group());
            return n;
        }
        if (name == "text" || name == "mathrm" || name == "textrm" || name == "operatorname" ||
            name == "mbox") {
            std::string content = read_raw_group();
            return make_atom(content, StyleUpright, 0.1f, 0.1f);
        }
        if (name == "mathbf" || name == "boldsymbol" || name == "bm") {
            std::string content = read_raw_group();
            return make_atom(content, StyleBold, 0.1f, 0.1f);
        }
        if (name == "mathit") {
            std::string content = read_raw_group();
            return make_atom(content, StyleItalic);
        }
        if (name == "hat" || name == "widehat" || name == "bar" || name == "overline" ||
            name == "vec" || name == "dot" || name == "ddot" || name == "tilde" ||
            name == "widetilde" || name == "check" || name == "breve" || name == "acute" ||
            name == "grave") {
            Node n;
            n.kind = Node::Kind::Accent;
            n.accent_type = name;
            n.children.push_back(parse_group());
            return n;
        }
        if (name == "left" || name == "right") {
            skip_spaces();
            if (i < src.size()) {
                char c = src[i++];
                if (c == '\\') {
                    std::string d = read_command_name();
                    std::string mapped = d;
                    if (d == "langle") mapped = "\xE2\x9F\xA8";
                    else if (d == "rangle") mapped = "\xE2\x9F\xA9";
                    else if (d == "lbrace") mapped = "{";
                    else if (d == "rbrace") mapped = "}";
                    else if (d == "|") mapped = "\xE2\x88\xA5";
                    return make_atom(mapped, StyleUpright, 0.05f, 0.05f);
                }
                std::string mapped(1, c);
                if (c == '.') return make_atom("", StyleUpright);
                if (c == '|') mapped = "\xE2\x88\xA5";
                return make_atom(mapped, StyleUpright, 0.05f, 0.05f);
            }
            return make_atom("", StyleUpright);
        }
        if (name == "bigl" || name == "bigr" || name == "Bigl" || name == "Bigr" ||
            name == "biggl" || name == "biggr" || name == "Biggl" || name == "Biggr") {
            skip_spaces();
            if (i < src.size()) {
                char c = src[i++];
                return make_atom(std::string(1, c), StyleUpright, 0.05f, 0.05f);
            }
            return make_atom("", StyleUpright);
        }
        if (name == "limits" || name == "nolimits" || name == "displaystyle" || name == "textstyle" ||
            name == "scriptstyle" || name == "scriptscriptstyle" || name == "!" || name == ",") {
            return make_atom("", StyleUpright);
        }
        if (name == ";") return make_atom("", StyleUpright, 0.22f, 0.0f);
        if (name == ":") return make_atom("", StyleUpright, 0.28f, 0.0f);
        if (name == " ") return make_atom("", StyleUpright, 0.17f, 0.0f);
        if (name == "quad") return make_atom("", StyleUpright, 1.0f, 0.0f);
        if (name == "qquad") return make_atom("", StyleUpright, 2.0f, 0.0f);
        if (name == "mod" || name == "bmod" || name == "pmod") {
            Node row;
            row.kind = Node::Kind::Row;
            row.children.push_back(make_atom("mod", StyleUpright, 0.16f, 0.16f));
            return row;
        }
        if (name == "lim" || name == "liminf" || name == "limsup" || name == "sup" || name == "inf" ||
            name == "max" || name == "min" || name == "argmax" || name == "argmin") {
            return make_atom(name, StyleUpright, 0.06f, 0.1f);
        }
        if (is_function_name(name)) {
            float space = function_table().at(name);
            return make_atom(name, StyleUpright, 0.0f, space);
        }
        auto it = symbol_table().find(name);
        if (it != symbol_table().end()) {
            const SymbolEntry& e = it->second;
            std::string text;
            append_utf8(text, e.cp);
            return make_atom(text, e.style, e.space_before, e.space_after);
        }
        if (name.empty()) return make_atom("", StyleUpright);
        fail("unknown command: \\" + name);
        return make_atom("", StyleUpright);
    }

    Node parse_row(bool top) {
        Node row;
        row.kind = Node::Kind::Row;
        while (i < src.size()) {
            char c = peek();
            if (c == '\0') break;
            if (c == '}') {
                if (top) {
                    fail("unexpected closing brace");
                    ++i;
                }
                break;
            }
            if (c == '^' || c == '_') {
                ++i;
                Node sub = parse_single();
                if (!failed) {
                    Node n;
                    n.kind = c == '^' ? Node::Kind::Sup : Node::Kind::Sub;
                    n.children.push_back(std::move(sub));
                    row.children.push_back(std::move(n));
                }
                continue;
            }
            Node n = parse_single();
            if (failed) break;
            if (n.kind == Node::Kind::Atom && n.text.empty() && n.space_before == 0.0f && n.space_after == 0.0f) continue;
            row.children.push_back(std::move(n));
        }
        if (row.children.empty()) row.children.push_back(make_atom("", StyleUpright));
        if (row.children.size() == 1 && top) return row.children[0];
        return row;
    }
};

struct Metrics {
    float width = 0.0f;
    float ascent = 0.0f;
    float descent = 0.0f;
};

const Font& font_for(int style, bool italic) {
    const auto& book = render::FontBook::instance();
    if (style == StyleBold && book.has("sans-bold")) return book.get("sans-bold");
    if (style == StyleItalic || italic) {
        if (book.has("sans-italic")) return book.get("sans-italic");
    }
    return book.default_font();
}

Metrics layout(const Node& n, float px);
Metrics layout_children_row(const std::vector<Node>& children, float px) {
    Metrics m;
    for (const auto& c : children) {
        Metrics cm = layout(c, px);
        m.width += cm.width + c.space_before * px;
        m.ascent = std::max(m.ascent, cm.ascent);
        m.descent = std::max(m.descent, cm.descent);
    }
    return m;
}

Metrics layout(const Node& n, float px) {
    Metrics m;
    switch (n.kind) {
        case Node::Kind::Atom: {
            float size = px * n.scale;
            const Font& f = font_for(n.style, n.style == StyleItalic);
            m.width = f.advance(n.text, size) + n.space_before * px + n.space_after * px;
            m.ascent = f.ascent(size);
            m.descent = f.descent(size);
            return m;
        }
        case Node::Kind::Row:
            return layout_children_row(n.children, px);
        case Node::Kind::Frac: {
            Metrics num = layout(n.children[0], px);
            Metrics den = layout(n.children[1], px);
            float gap = px * 0.12f;
            float rule = std::max(1.0f, px * 0.045f);
            float axis = px * 0.32f;
            m.width = std::max(num.width, den.width) + px * 0.3f;
            m.ascent = axis + gap + rule * 0.5f + num.ascent + num.descent;
            m.descent = -axis + gap + rule * 0.5f + den.ascent + den.descent;
            m.descent = std::max(m.descent, 0.0f);
            return m;
        }
        case Node::Kind::Root: {
            bool has_degree = !n.children[0].text.empty() || !n.children[0].children.empty();
            Metrics body = layout(n.children[1], px);
            float h = body.ascent + body.descent;
            float sw = h * 0.38f + px * 0.1f;
            m.width = sw + body.width;
            m.ascent = body.ascent + px * 0.12f;
            m.descent = body.descent;
            if (has_degree) {
                Metrics deg = layout(n.children[0], px * 0.55f);
                m.ascent += deg.ascent + deg.descent + px * 0.05f;
                m.width = std::max(m.width, deg.width + sw + body.width);
            }
            return m;
        }
        case Node::Kind::Sup: {
            Metrics c = layout(n.children[0], px * 0.7f);
            float shift = px * 0.42f;
            m.width = c.width;
            m.ascent = shift + c.ascent;
            m.descent = std::max(0.0f, c.descent - shift);
            return m;
        }
        case Node::Kind::Sub: {
            Metrics c = layout(n.children[0], px * 0.7f);
            float shift = px * 0.26f;
            m.width = c.width;
            m.ascent = std::max(0.0f, c.ascent - shift);
            m.descent = shift + c.descent;
            return m;
        }
        case Node::Kind::Accent: {
            Metrics c = layout(n.children[0], px);
            m.width = c.width;
            m.ascent = c.ascent + px * 0.5f;
            m.descent = c.descent;
            return m;
        }
    }
    return m;
}

void draw_node(render::Canvas& canvas, const Node& n, float x, float y, float px, Color color);
void draw_row(render::Canvas& canvas, const Node& n, float x, float y, float px, Color color) {
    float cx = x;
    for (const auto& c : n.children) {
        cx += c.space_before * px;
        Metrics m = layout(c, px);
        draw_node(canvas, c, cx, y, px, color);
        cx += m.width;
    }
}

void draw_accent(render::Canvas& canvas, const std::string& type, float x, float top, float w, float px, Color color) {
    float width = w > px * 0.15f ? w : px * 0.45f;
    float thickness = std::max(1.0f, px * 0.055f);
    float y0 = top - px * 0.08f;
    if (type == "bar" || type == "overline") {
        canvas.draw_line({x, y0}, {x + width, y0}, color, thickness);
    } else if (type == "hat" || type == "widehat") {
        canvas.draw_line({x + width * 0.1f, y0 - px * 0.28f}, {x + width * 0.5f, y0}, color, thickness);
        canvas.draw_line({x + width * 0.5f, y0}, {x + width * 0.9f, y0 - px * 0.28f}, color, thickness);
    } else if (type == "vec") {
        float my = y0 - px * 0.14f;
        canvas.draw_line({x + width * 0.1f, my}, {x + width * 0.9f, my}, color, thickness);
        canvas.draw_line({x + width * 0.72f, my - px * 0.09f}, {x + width * 0.9f, my}, color, thickness);
        canvas.draw_line({x + width * 0.72f, my + px * 0.09f}, {x + width * 0.9f, my}, color, thickness);
    } else if (type == "dot" || type == "acute" || type == "grave") {
        canvas.draw_circle({x + width * 0.5f, y0 - px * 0.14f}, std::max(1.2f, px * 0.06f), color);
    } else if (type == "ddot") {
        canvas.draw_circle({x + width * 0.36f, y0 - px * 0.14f}, std::max(1.2f, px * 0.055f), color);
        canvas.draw_circle({x + width * 0.64f, y0 - px * 0.14f}, std::max(1.2f, px * 0.055f), color);
    } else if (type == "tilde" || type == "widetilde") {
        std::vector<Vec2> pts;
        int npts = 14;
        for (int i = 0; i <= npts; ++i) {
            float t = static_cast<float>(i) / npts;
            pts.push_back({x + width * t, y0 - px * 0.14f - std::sin(t * 6.2831853f) * px * 0.09f});
        }
        canvas.draw_polyline(pts, color, thickness);
    } else if (type == "check") {
        canvas.draw_line({x + width * 0.1f, y0 - px * 0.28f}, {x + width * 0.5f, y0}, color, thickness);
        canvas.draw_line({x + width * 0.5f, y0}, {x + width * 0.9f, y0 - px * 0.28f}, color, thickness);
    } else if (type == "breve") {
        canvas.draw_line({x + width * 0.15f, y0 - px * 0.22f}, {x + width * 0.5f, y0}, color, thickness);
        canvas.draw_line({x + width * 0.5f, y0}, {x + width * 0.85f, y0 - px * 0.22f}, color, thickness);
    }
}

void draw_node(render::Canvas& canvas, const Node& n, float x, float y, float px, Color color) {
    switch (n.kind) {
        case Node::Kind::Atom: {
            if (n.text.empty()) return;
            float size = px * n.scale;
            const Font& f = font_for(n.style, n.style == StyleItalic);
            canvas.draw_text({x + n.space_before * px, y}, n.text, f, size, color);
            return;
        }
        case Node::Kind::Row:
            draw_row(canvas, n, x, y, px, color);
            return;
        case Node::Kind::Frac: {
            float gap = px * 0.12f;
            float rule = std::max(1.0f, px * 0.045f);
            float axis = px * 0.32f;
            Metrics num = layout(n.children[0], px);
            Metrics den = layout(n.children[1], px);
            float w = std::max(num.width, den.width) + px * 0.3f;
            float rule_y = y - axis;
            canvas.draw_line({x, rule_y}, {x + w, rule_y}, color, rule);
            float num_x = x + (w - num.width) * 0.5f;
            float num_y = rule_y - gap - num.descent;
            draw_node(canvas, n.children[0], num_x, num_y, px, color);
            float den_x = x + (w - den.width) * 0.5f;
            float den_y = rule_y + gap + den.ascent;
            draw_node(canvas, n.children[1], den_x, den_y, px, color);
            return;
        }
        case Node::Kind::Root: {
            const Node& deg_node = n.children[0];
            const Node& body = n.children[1];
            bool has_degree = !deg_node.text.empty() || !deg_node.children.empty();
            Metrics bm = layout(body, px);
            float h = bm.ascent + bm.descent;
            float sw = h * 0.38f + px * 0.1f;
            float top = y - bm.ascent;
            float thickness = std::max(1.0f, px * 0.06f);
            std::vector<Vec2> sign = {
                {x + 0.03f * h, top + 0.40f * h},
                {x + 0.15f * h, top + 0.58f * h},
                {x + sw, top},
            };
            canvas.draw_polyline(sign, color, thickness);
            canvas.draw_line({x + sw, top}, {x + sw + bm.width + px * 0.04f, top}, color, thickness);
            draw_node(canvas, body, x + sw, y, px, color);
            if (has_degree) {
                Metrics dm = layout(deg_node, px * 0.55f);
                float deg_y = top - px * 0.05f - dm.descent;
                draw_node(canvas, deg_node, x + sw - dm.width * 0.4f, deg_y, px * 0.55f, color);
            }
            return;
        }
        case Node::Kind::Sup: {
            draw_node(canvas, n.children[0], x, y - px * 0.42f, px * 0.7f, color);
            return;
        }
        case Node::Kind::Sub: {
            draw_node(canvas, n.children[0], x, y + px * 0.26f, px * 0.7f, color);
            return;
        }
        case Node::Kind::Accent: {
            const Node& body = n.children[0];
            Metrics bm = layout(body, px);
            draw_node(canvas, body, x, y, px, color);
            draw_accent(canvas, n.accent_type, x, y - bm.ascent, bm.width, px, color);
            return;
        }
    }
}
}

struct Equation::Impl {
    Node root;
    bool valid = false;
    std::string error;
};

Equation::Equation(): m_impl(std::make_shared<Impl>()) {}
Equation::Equation(const std::string& latex): m_impl(std::make_shared<Impl>()) {
    Parser p(latex);
    m_impl->root = p.parse_row(true);
    if (p.failed) {
        m_impl->valid = false;
        m_impl->error = p.error;
    } else {
        m_impl->valid = true;
    }
}

Equation::Equation(const Equation& other) = default;
Equation::Equation(Equation&& other) noexcept = default;
Equation& Equation::operator=(const Equation& other) = default;
Equation& Equation::operator=(Equation&& other) noexcept = default;
Equation::~Equation() = default;
bool Equation::valid() const { return m_impl->valid; }
const std::string& Equation::error() const { return m_impl->error; }
Layout Equation::measure(float px) const {
    Layout out;
    if (!m_impl->valid || px <= 0.0f) return out;
    Metrics m = layout(m_impl->root, px);
    out.width = m.width;
    out.ascent = m.ascent;
    out.descent = m.descent;
    return out;
}

void Equation::draw(render::Canvas& canvas, render::Vec2 baseline_left, float px, render::Color color) const {
    if (!m_impl->valid || px <= 0.0f) return;
    draw_node(canvas, m_impl->root, baseline_left.x, baseline_left.y, px, color);
}

void Equation::draw_aligned(render::Canvas& canvas, render::Vec2 anchor, float px, render::Color color, render::TextAlign align) const {
    if (!m_impl->valid || px <= 0.0f) return;
    Layout m = measure(px);
    float x = anchor.x;
    if (align == TextAlign::Center) x -= m.width * 0.5f;
    else if (align == TextAlign::Right) x -= m.width;
    draw(canvas, render::Vec2{x, anchor.y}, px, color);
}

Layout measure(const std::string& latex, float px) { return Equation(latex).measure(px); }
void draw(render::Canvas& canvas, render::Vec2 baseline_left, const std::string& latex, float px, render::Color color) {
    Equation eq(latex);
    if (!eq.valid()) throw ParseError("equation parse error: " + eq.error());
    eq.draw(canvas, baseline_left, px, color);
}

void draw_aligned(render::Canvas& canvas, render::Vec2 anchor, const std::string& latex, float px, render::Color color, render::TextAlign align) {
    Equation eq(latex);
    if (!eq.valid()) throw ParseError("equation parse error: " + eq.error());
    eq.draw_aligned(canvas, anchor, px, color, align);
}
}
}
