#include <utility>

#ifndef CALCULATOR_CALCASTEXCEPTION_HPP
#define CALCULATOR_CALCASTEXCEPTION_HPP

#include <string>

#include "Token.hpp"

#include "boilerplate/ld_boilerplate.hpp"

/**
 * An exception that is raised when the calculator just doesn't like something.
 *
 * This can be raised by any step in reality - tokenization step, AST
 * generation, or actual execution. Where it raises doesn't matter - what
 * matters is the token that _raised_ the exception, which this class
 * conveniently stores for any from of pretty-printing that you'd like to
 * perform.
 */
class CalcASTException : public std::exception {
	/**
	 * The token that caused the exception
	 */
	Token token;

	/**
	 * A user-friendly message that specifies what went wrong
	 */
	std::wstring msg;

	public:
	CalcASTException(Token token, std::wstring msg) : token(std::move(token)),
	                                                  msg(std::move(msg)) {}

	/**
	 * Gets the token that caused the exception.
	 *
	 * @return The token
	 */
	Token get_token() const {
		return token;
	}

	/**
	 * Gets the user-friendly message that caused the exception.
	 *
	 * @return
	 */
	std::wstring get_msg() const {
		return msg;
	}

	/**
	 * Don't use this. It returns a C string, which is no fun. :<
	 *
	 * @return The boring C string version of [[CalcASTException::get_msg]].
	 */
	const char * what() const noexcept {
		return LD::w2str(msg).c_str();
	}
};

#endif //CALCULATOR_CALCASTEXCEPTION_HPP
