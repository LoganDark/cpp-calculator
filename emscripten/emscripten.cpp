#ifndef __EMSCRIPTEN__
	#include "/Users/LoganDark/emsdk/emscripten/1.38.20/system/include/emscripten.h"
	#include "/Users/LoganDark/emsdk/emscripten/1.38.20/system/include/emscripten/html5.h"
	#include "/Users/LoganDark/emsdk/emscripten/1.38.20/system/include/emscripten/bind.h"
	#include "/Users/LoganDark/emsdk/emscripten/1.38.20/system/include/emscripten/val.h"
#else
	#include <emscripten.h>
	#include <emscripten/html5.h>
	#include <emscripten/bind.h>
	#include <emscripten/val.h>
#endif

#include "../RationalCalculator.cpp"

#define EXC_WRAPPER(x)                                                         \
    try {                                                                      \
        x                                                                      \
    } catch (CalcASTException & exc) {                                         \
        emscripten::val(exc).throw_();                                         \
    } catch (std::exception & etc) {                                           \
        emscripten::val::global("console")                                     \
            .call<void>("error", emscripten::val(LD::c2wstr(etc.what())));     \
    }

class EmscriptenRationalCalculator : public RationalCalculator {
	public:
	std::vector<Token> last_tokens;

	std::vector<Token> _tokenize(const std::wstring & input) {
		CalcTokenizer<math::Rational> tokenizer(input);

		try {
			last_tokens = tokenizer.tokenize();

			return last_tokens;
		} catch (CalcASTException & exc) {
			last_tokens = tokenizer.tokenize();

			emscripten::val(exc).throw_();
		} catch (std::exception & etc) {
			last_tokens = tokenizer.tokenize();

			emscripten::val::global("console")
				.call<void>("error", emscripten::val(
					LD::c2wstr(etc.what())));
		}
	}

	CalcASTElem _get_ast(const std::vector<Token> & tokens) {
		EXC_WRAPPER(
			return get_ast(tokens);
		)
	}

	/**
	 * @return A JavaScript object representing all the variables of the
	 * calculator. Keys are the variable names, values are
	 * [[RationalCalculator::to_string]]'d numbers. These aren't meant for
	 * modification.
	 */
	emscripten::val get_vars() {
		emscripten::val obj = emscripten::val::object();

		auto end = variables.end();

		for (auto it = variables.begin(); it != end; it++) {
			obj.set(it->first, to_string(it->second));
		}

		return obj;
	}

	std::wstring _execute_ast(const CalcASTElem & ast) {
		EXC_WRAPPER(
			return to_string(execute_ast(ast, false, true));
		)
	}

	bool valid_ast(const CalcASTElem & ast) {
		EXC_WRAPPER(
			execute_ast(ast, true);

			return true;
		)
	}

	std::vector<Token> _tokenize_command(const std::wstring & input) {
		EXC_WRAPPER(
			return tokenize_command(input);
		)
	}

	std::wstring _execute_command(std::vector<Token> tokens) {
		EXC_WRAPPER(
			return execute_command(std::move(tokens));
		)
	}

	bool validate_command(std::vector<Token> tokens) {
		EXC_WRAPPER(
			execute_command(std::move(tokens), true);

			return true;
		)
	}

	EmscriptenRationalCalculator() : RationalCalculator() {}
};

EMSCRIPTEN_BINDINGS(tokens) {
	emscripten::enum_<TokenType>("TokenType")
		.value("UNKNOWN", TK_UNKNOWN)
		.value("NUMBER", TK_NUMBER)
		.value("OPERATOR", TK_OPERATOR)
		.value("UOPERATOR", TK_UOPERATOR)
		.value("OPAREN", TK_OPAREN)
		.value("CPAREN", TK_CPAREN)
		.value("FOPAREN", TK_FOPAREN)
		.value("FCPAREN", TK_FCPAREN)
		.value("VARIABLE", TK_VARIABLE)
		.value("FUNCTION", TK_FUNCTION)
		.value("COMMA", TK_COMMA);

	emscripten::value_object<Token>("Token")
		.field("type", & Token::type)
		.field("data", & Token::data)
		.field("pos", & Token::pos);

	emscripten::register_vector<Token>("vector<Token>");
}

EMSCRIPTEN_BINDINGS(ast) {
	emscripten::class_<CalcASTElem>("CalcASTElem")
		.constructor<int, Token, std::vector<CalcASTElem>>()
		.property("type", & CalcASTElem::type)
		.property("token", & CalcASTElem::token)
		.property("children", & CalcASTElem::children);

	emscripten::class_<CalcASTException>("CalcASTException")
		.constructor<Token, std::wstring>()
		.function("get_token", & CalcASTException::get_token)
		.function("get_msg", & CalcASTException::get_msg);

	emscripten::register_vector<CalcASTElem>("vector<CalcASTElem>");

}

EMSCRIPTEN_BINDINGS(calculator) {
	emscripten::class_<EmscriptenRationalCalculator>("RationalCalculator")
		.constructor()
			// tokenization
		.property("last_tokens",
		          & EmscriptenRationalCalculator::last_tokens)
		.function("tokenize",
		          & EmscriptenRationalCalculator::_tokenize)

			// AST
		.function("get_ast",
		          & EmscriptenRationalCalculator::_get_ast)
		.function("execute_ast",
		          & EmscriptenRationalCalculator::_execute_ast)
		.function("valid_ast",
		          & EmscriptenRationalCalculator::valid_ast)

			// metadata
		.function("get_vars",
		          & EmscriptenRationalCalculator::get_vars)

			// commands
		.function("tokenize_command",
		          & EmscriptenRationalCalculator::_tokenize_command)
		.function("execute_command",
		          & EmscriptenRationalCalculator::_execute_command)
		.function("validate_command",
		          & EmscriptenRationalCalculator::validate_command);
}

int main() {
	emscripten::val::global("window").call<void>("after_init");
}