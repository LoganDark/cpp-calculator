#ifndef CALCULATOR_CALCTOKENIZER_HPP
#define CALCULATOR_CALCTOKENIZER_HPP

#include <string>
#include <vector>

#include "Tokenizer.cpp"
#include "CalcTkEnum.hpp"
#include "CalcOperator.hpp"
#include "CalcOperators.hpp"

#define STRING_CONTAINS(str, test) ((str).find_first_of(test) != std::wstring::npos)
#define CONFORMS(str, test) ((str).find_first_not_of(test) == std::wstring::npos)

template <class Num>
	class CalcTokenizer : public Tokenizer {
		/**
		 * Returns true if there's an operator starting with the specified
		 * string.
		 *
		 * @tparam Op Operator type
		 * @param ops Operator vector
		 * @param prefix Char to search for
		 * @return If there's a match
		 */
		template <class Op>
			bool ops_start_with(std::vector<Op> & ops,
			                    std::wstring prefix);

		/**
		 * Returns true if there's an operator starting with the specified
		 * string.
		 *
		 * @tparam Op Operator type
		 * @param ops Operator vector
		 * @param prefix Char to search for
		 * @return If there's a match
		 */
		template <class Op>
			bool ops_start_with(std::vector<Op> & ops,
			                    wchar_t prefix);

		/**
		 * Consumes all characters from string `str`. See
		 * [[CalcTokenizer::consume_push]] for a function that creates a new token.
		 *
		 * @param str The string to consume characters from.
		 */
		void consume(const std::wstring & str);

		/**
		 * Does all the main work, honestly. Uses the various [[Tokenizer]]
		 * functions to chop up a string into little itty bitty bits and label them
		 * based on an extremely strict set of rules.
		 */
		void _tokenize() override;

		/**
		 * @return Whether the last token is a value. For example if the token
		 * stream is
		 *
		 * ```
		 * (NUMBER   ) 5
		 * (UOPERATOR) !
		 * ```
		 *
		 * it will return true. If it's
		 *
		 * ```
		 * (NUMBER   ) 5
		 * (OPERATOR ) -
		 * ```
		 *
		 * it will return false.
		 */
		bool after_value();

		/**
		 * @return true if the last token is an opening parenthesis
		 */
		bool after_operator();

		/**
		 * @return `true` only if we're after a unary operator that comes BEFORE
		 * a value
		 */
		bool after_uop();

		/**
		 * @param op
		 * @return the associacivity of unary operator `op`
		 */
		UnaryAssociativity uassociativity(std::wstring op);

		/**
		 * Pushes a token that isn't actually in the input stream, but is
		 * required for e.g. the shunting-yard / AST generation. For example,
		 * implied multiplication in 5(4).
		 *
		 * @param type The type of token to push.
		 * @param op The contents of the token.
		 */
		void push_implied_token(TokenType type, std::wstring op);

		/**
		 * @return If we are immediately before a value
		 */
		bool immediately_before_value();

		/**
		 * Tries to find the last token of type `type`, if not, throws a
		 * [[CalcASTException]] with the error string `error`.
		 *
		 * This function works backwards on the current tokens. It finds the
		 * token which changes the scope to `0`. The scope being positive, for
		 * example, means it'll return the `scope`th token. `1`, for example,
		 * would find the opening token that corresponds to the closing token
		 * (assumed not pushed to the stack).
		 *
		 * @param opening 'Opening' token (decrements scope)
		 * @param closing 'Closing' token (increments scope)
		 * @return The found token, or a blank token (type `TK_UNKNOWN`) if not
		 * found
		 */
		Token get_matching_token(TokenType opening, TokenType closing,
		                         int scope);

		public:
		/**
		 * All characters that can be used in variables
		 */
		static std::wstring var_chars;

		/**
		 * All characters that make up a number
		 *
		 * I know that `.` is included in this and that it may show up multiple
		 * times, but the execution layer (after AST) will catch this
		 */
		static std::wstring digits;

		/**
		 * Binary operators
		 */
		static std::vector<CalcOperator<Num>> binary_ops;

		/**
		 * Unary operators
		 */
		static std::vector<CalcUOperator<Num>> unary_ops;

		/**
		 * I'll leave figuring out what this is as an exercise for the reader
		 */
		static std::wstring parenthesis;

		explicit CalcTokenizer(const std::wstring & str);
	};

#endif //CALCULATOR_CALCTOKENIZER_HPP
