#ifndef CALCULATOR_CALCULATOR_HPP
#define CALCULATOR_CALCULATOR_HPP

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "CalcASTElem.hpp"
#include "CalcOpFunc.hpp"

/**
 * A CalcCommand stores a function that can be called as a command for the
 * calculator.
 */
typedef std::function<std::wstring(std::vector<Token>, bool)> CalcCommand;

/**
 * Calculator. Stores the default configuration for a prompt that allows the
 * user to enter expressions and have them evaluated. Includes some commands
 * built-in to control precision, get help, etc.
 *
 * It also stores state, like the variables defined (which stores the last
 * result as well).
 *
 * @tparam Num The number type to use. This won't work well for things other
 * than [[math::Rational]] or compatible classes unless you override
 * [[Calculator::is_int]] and [[Calculator::to_string]] to work with your type
 * (with something like a subclass or just a modification to this file).
 */
template <class Num>
	class Calculator {
		protected:
		/**
		 * Whether the calculator should show tokenizer data.
		 *
		 * This is usually always turned off, but subclasses can change it.
		 */
		bool tk_debug = false;

		/**
		 * Force token information to be printed before AST generation
		 */
		bool tk_debug_force = false;

		/**
		 * Whether the calculator should show AST data as well.
		 *
		 * This is usually always turned off, but subclasses can change it.
		 */
		bool ast_debug = false;

		/**
		 * Force AST information to be printed before evaluation
		 */
		bool ast_debug_force = false;

		/**
		 * Tokenizes a string. Internally just uses [[CalcTokenizer::tokenize]].
		 *
		 * This function can throw [[CalcASTException]]s.
		 *
		 * @param input The string to tokenize.
		 * @return A linear list of tokens extracted from the input.
		 */
		std::vector<Token> tokenize(const std::wstring & input);

		/**
		 * Executes a command `cmd` with args `args` and returns the result.
		 * This function basically looks into [[Calculator::commands]] and
		 * either executes what it can find or returns "Unknown command."
		 *
		 * @param cmd The command to attempt to execute.
		 * @param args The arguments to execute it with.
		 * @return The output of the command, without a trailing newline (\n).
		 */
		std::wstring execute_command(std::vector<Token> args,
		                             bool validate_only = false);

		/**
		 * Gets the AST (Abstract Syntax Tree) from a given list of tokens. Can
		 * throw [[CalcASTException]]s.
		 *
		 * Handles implicit operations (i.e. "* 5" -> "_ * 5" which means
		 * "multiply the last answer by 5"), as well as throwing a
		 * [[CalcASTException]] if there is no last answer to use, but otherwise
		 * just uses [[CalcAST::generate_ast]].
		 *
		 * @param tokens The tokens to get the AST of.
		 * @return The AST generated from those tokens.
		 */
		CalcASTElem get_ast(std::vector<Token> tokens);

		public:
		/**
		 * The variables. Each variable can only consist of a-z, A-Z, and _. See
		 * CalcTokenizer.hpp.
		 */
		std::map<std::wstring, Num>                                 variables;

		/**
		 * Functions the calculator can execute.
		 *
		 * CalcOpFunc<Num> functions
		 *                     .at(std::string function_name)
		 *                     .at(unsigned function_arity)
		 *
		 * Variadic functions are not supported at the moment.
		 *
		 * @formatter:off
		 */
		std::map<std::wstring, std::map<unsigned, CalcOpFunc(Num)>> functions;
		std::map<std::wstring, CalcOpFunc(Num)> variadic_funcs;
		// @formatter:on

		/**
		 * Stores all the help pages. Self-explanatory.
		 */
		std::map<std::wstring, std::wstring> help_pages {
			{
				L"starthere", L"Hello there, you amazing calculator person!"
				              L" Would you like to use the BEST CALCULATOR IN"
				              L" EXISTENCE?... Well, unfortunately, this isn't"
				              L" it.\n"
				              L"\n"
				              L"There are multiple help pages - in order,"
				              L" they're:\n"
				              L":help basics\n"
				              L":help pemdas\n"
				              L":help variables\n"
				              L":help operations"
			}, {
				L"basics", L"This calculator supports basic operations like 2 +"
				           L" 2. The result of every evaluation is stored in a"
				           L" variable called _ (one underscore). If your"
				           L" expression starts with an operator (i.e. * 3)"
				           L" then that variable will be used (the calculator"
				           L" will act as if that was _ * 3). You can of course"
				           L" use _ in place of any other number in your"
				           L" expression, as it is a normal variable. See "
				           L" :help variables for more information on"
				           L" variables."
			}, {
				L"pemdas", L"This calculator supports the order of operations."
				           L" For example, 5 + 3 * 5 == 20, not 40. The"
				           L" calculator does not interpret it as (5 + 3) * 5."
			}, {
				L"variables", L"This calculator supports variables. You can"
				              L" assign variables using the = operator."
				              L" Assigning a variable returns what you assigned"
				              L" to it. For example, (x = 3) + 5 will return 8"
				              L" but x will be set to 3. You can use a variable"
				              L" wherever a number is accepted."
			}, {
				L"operations", L"Operations supported by this calculator:\n"
				               "Parenthesis: (2 + 2) * 2 = 8\n"
				               "Exponentiation: 2 ^ 2 = 4\n"
				               "Multiplication: 2 * 2 = 4\n"
				               "Division: 2 / 2 = 1\n"
				               "Addition: 2 + 2 = 4\n"
				               "Subtraction: 2 - 2 = 0\n"
				               "Negation: -(2 + 2) = -4\n"
				               "Useless: +(2 + 2) = 4\n"
				               "Factorial: 5! = 120\n"
				               "Double factorial: 5!! = 15\n"
				               "Superfactorial: 5$ = 34560"
			}
		};

		/**
		 * Stores all commands that can be executed. See [[CalcCommand]]
		 */
		std::map<std::wstring, CalcCommand> commands;

		/**
		 * Constructor. Initializes the help command, but does nothing else.
		 */
		Calculator();

		/**
		 * Converts a string to a list of tokens and a command name for use with
		 * [[Calculator::execute_command]].
		 *
		 * @param input
		 * @return
		 */
		std::vector<Token> tokenize_command(const std::wstring & input);

		/**
		 * Execute a command. Takes input in the form of "command arg1 arg2" -
		 * WITHOUT the leading ':'. Splits the arguments up and sends them to
		 * [[Calculator::execute_command]].
		 *
		 * @param input The command to execute
		 * @return The output of the command
		 */
		std::wstring execute_command_str(const std::wstring & input,
		                                 bool validate_only = false);

		/**
		 * Joins 2 tokens together. Token 2 will take priority over 1 in case of
		 * an overlap, and the string will be padded with spaces in blank spots.
		 *
		 * Token 2 is basically overlaid over 1. Token 2's type will be used for
		 * the result.
		 *
		 * @param tk1
		 * @param tk2
		 * @return
		 */
		Token join_tokens(const Token & tk1, const Token & tk2);

		/**
		 * Gets a full token that is equivalent to the AST element `ast`. For
		 * example, this could be used to retrieve a full expression from an AST
		 * for purposes such as raising an exception for an entire side of an
		 * operation.
		 *
		 * @param ast The AST to get a token of
		 * @return
		 */
		Token get_token(const CalcASTElem & ast);

		/**
		 * The meat of the calculator. This is where the calculations happen.
		 *
		 * Accepts an AST from i.e. [[Calculator::get_ast]] and returns the
		 * result from evaluating it. Does variable assignments, resolves
		 * variable names, does all the operations and also includes incredibly
		 * inefficient exponentiation. Amazing stuff...
		 *
		 * It's recursive, so you just call it for whatever element you need. It
		 * will automatically call itself for all other parts of the expression.
		 *
		 * @param ast The AST to execute.
		 * @return The evaluated result.
		 */
		Num execute_ast(const CalcASTElem & ast, bool validate_only = false,
		                bool set_ = false);

		/**
		 * Generates a string from a token showing its type and data.
		 *
		 * @param tk The token to stringify
		 * @return
		 */
		std::wstring stringify_tk(const Token & tk);

		/**
		 * Run [[Calculator::debug_token]] for every element in `tokens`.
		 *
		 * @param tokens The tokens to stringify
		 * @return
		 */
		std::wstring stringify_tks(const std::vector<Token> & tokens);

		/**
		 * Generates a user-friendly debug string from an AST.
		 *
		 * @param ast The AST to stringify
		 * @param indent Used internally for recursion purposes
		 * @return
		 */
		std::wstring stringify_ast(const CalcASTElem & ast,
		                           const std::wstring & newline = L"\n");

		/**
		 * Executes a string. If it's empty, return nothing. If it's a command,
		 * execute that command. If it's not, execute an expression. Return the
		 * result of any of those as a string. [[Calculator::execute_ast]] on
		 * its own can return a number value directly, but this function only
		 * returns a string.
		 *
		 * This function also takes into account [[Calculator::tk_debug]]
		 * and [[Calculator::ast_debug]]. It appends the output of
		 * [[Calculator::stringify_tks]] and [[Calculator::stringify_ast]], in
		 * that order, if the corresponding debug variables are set to `true`.
		 *
		 * @param input The input to execute.
		 * @return The output.
		 */
		std::wstring execute(const std::wstring & input,
		                     bool validate_only = false);

		/**
		 * Executes multiple things separated by semicolons, using
		 * [[Calculator::execute]] for each one. Because why not?
		 *
		 * @param input The input.
		 * @return The list of results from each [[Calculator::execute]] call.
		 */
		std::vector<std::wstring> execute_multiple(const std::wstring & input,
		                                           bool validate_only = false);

		/**
		 * Check if something is integer. This is its own function because
		 * our arbitrary-precision type doesn't have modulo like normal numbers
		 * do. This must be overridden.
		 *
		 * @param num The number to check.
		 * @return If it's an int.
		 */
		virtual bool is_int(const Num & num) = 0;

		/**
		 * Converts a number to a string. This must be overridden.
		 *
		 * @param num The number to convert.
		 * @return The string
		 */
		virtual std::wstring to_string(const Num & num) = 0;
	};

#endif //CALCULATOR_CALCULATOR_HPP
