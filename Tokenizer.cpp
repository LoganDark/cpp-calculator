#ifndef CALCULATOR_TOKENIZER_CPP
#define CALCULATOR_TOKENIZER_CPP

#include <string>
#include <vector>

#include "Token.hpp"

class Tokenizer {
	protected:
	std::wstring str;

	/**
	 * The current token stream. This is only populated by a _single_ call
	 * to [[Tokenizer::_tokenize]].
	 */
	std::vector<Token> tokens;

	/**
	 * Whether this tokenizer has been used. This prevents multiple calls to
	 * [[Tokenizer::_tokenize]] which would mess up [[Tokenizer::tokens]].
	 */
	bool used = false;

	/**
	 * Tokenizes the string. This doesn't return anything, it instead uses
	 * methods like [[Tokenizer::push_token]] to push tokens to
	 * [[Tokenizer::tokens]], which will be returned by
	 * [[Tokenizer::tokenize]].
	 *
	 * @return
	 */
	virtual void _tokenize() = 0;

	unsigned long pos = 0;

	/**
	 * The start of the current token. This is a pointer because there is a
	 * difference between 0 and no value
	 */
	unsigned long token_start = 0;
	bool token_started = false;

	/**
	 * Peeks `delta` characters from the current position. `0` returns the
	 * current character. Returns `-1` if we're at the end of the string.
	 *
	 * @param delta
	 * @return
	 */
	wchar_t peek(long delta = 0) {
		unsigned long index = pos + delta;

		if (index >= str.length()) {
			return -1;
		}

		return str[index];
	}

	/**
	 * Moves `delta` characters forward, without returning any characters.
	 * Can be used after `peek`.
	 *
	 * @param delta
	 */
	void move(long delta = 1) {
		pos = pos + delta;
	}

	/**
	 * Gets `delta` characters, returns them, and moves `delta` characters
	 * forward.
	 *
	 * @param delta
	 * @return
	 */
	std::wstring get(unsigned long delta = 1) {
		std::wstring substring = str.substr(pos, delta);
		move(delta);

		return substring;
	}

	/**
	 * Gets the current token data.
	 *
	 * @param delta
	 * @return
	 */
	std::wstring get_token_data() {
		return str.substr(token_start, pos - token_start);
	}

	/**
	 * Starts a token. The token type is specified in [[Tokenizer::push_token]]
	 */
	void start_token() {
		token_start = pos;
		token_started = true;
	}

	/**
	 * [[Tokenizer::start_token]] must be called prior to this. This ends the
	 * current token and pushes it to [[Tokenizer::tokens]].
	 *
	 * @param token_type The type of token to push.
	 */
	void push_token(int token_type) {
		if (!token_started) {
			throw std::runtime_error("Token doesn't have a start position");
		}

		/**
		 * If the token is more than 0 characters long...
		 */
		if (pos > token_start) {
			/**
			 * Get the contents of the token
			 */
			std::wstring data = get_token_data();

			/**
			 * Place a new, generated token in [[Tokenizer::tokens]]
			 */
			tokens.emplace_back(token_type, data, token_start);

			/**
			 * End the token
			 */
			token_started = false;
		}
	}

	/**
	 * This function consumes all whitespace from the current position and
	 * sets the current position to the next non-whitespace character.
	 *
	 * If `ws_type` is defined, it creates a token with that type containing
	 * the whitespace.
	 *
	 * @param ws_type The type to give whitespace tokens. If below 0, token
	 * will not be generated.
	 * @param inverse Whether to chomp non-whitespace characters instead.
	 */
	void chomp_whitespace(int ws_type = -1, bool inverse = false) {
		if (ws_type > -1) {
			start_token();
		}

		while (peek() > -1 &&
		       (peek() == 32 || std::isspace(peek())) == !inverse) {
			move();
		}

		if (ws_type > -1) {
			push_token(ws_type);
		}
	}

	public:
	/**
	 * Tokenizes the string and returns the token stream. Calls
	 * [[Tokenizer::_tokenize]] internally, but only on the first call. It
	 * then just copies [[Tokenizer::tokens]] and returns it.
	 *
	 * @return
	 */
	std::vector<Token> tokenize() {
		if (!used) {
			used = true;

			_tokenize();
		}

		return tokens;
	}

	/**
	 * Creates a new tokenizer. A tokenizer is a single-use object
	 * representing the tokenization of a string.
	 *
	 * @param str
	 */
	explicit Tokenizer(std::wstring str) : str(std::move(str)) {}
};

#endif //CALCULATOR_TOKENIZER_CPP
