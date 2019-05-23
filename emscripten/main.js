let highlight_exp,
    rehighlight,
    set_sel_range,
    get_sections,
    show_dialog,
    new_entry_group,
    add_to_history,
    highlight_val,
    escape_str

let con_buffer      = '',
    con_active      = false,
    /**
     * @type {Object<number, string>}
     */
    token_colors    = {},
    /**
     * @type {Object<number, string>}
     */
    token_lookup    = {},
    dialogs         = 1,
    history         = [],
    current_history = [''],
    history_index   = 0

/**
 * Define this before calc.js loads
 *
 * @type {{print : Window.Module.print}}
 */
window.Module = {
	'print' : function(str) {
		if (con_active) {
			con_buffer += str + '\n'
		}
	}
}

/**
 * this is called by calc.js
 */
function after_init() {
	token_colors[Module.TokenType.UNKNOWN.value] = ''
	token_colors[Module.TokenType.NUMBER.value] = 'text-purple'
	token_colors[Module.TokenType.OPERATOR.value] = 'text-grey'
	token_colors[Module.TokenType.UOPERATOR.value] = 'text-purple'
	token_colors[Module.TokenType.OPAREN.value] = 'text-blue'
	token_colors[Module.TokenType.CPAREN.value] = 'text-blue'
	token_colors[Module.TokenType.FOPAREN.value] = 'text-blue'
	token_colors[Module.TokenType.FCPAREN.value] = 'text-blue'
	token_colors[Module.TokenType.VARIABLE.value] = 'text-green'
	token_colors[Module.TokenType.FUNCTION.value] = 'text-blue'
	token_colors[Module.TokenType.COMMA.value] = 'text-blue'

	const dialog_template = document.getElementById('dialog')
	dialog_template.parentElement.removeChild(dialog_template)
	dialogs--
	dialog_template.removeAttribute('id')
	dialog_template.lastElementChild.lastElementChild.classList.remove('hidden')

	const calc = new Module.RationalCalculator()
	const input = document.getElementById('input')
	const err = document.getElementById('error-msg')
	const vars = document.getElementById('vars')
	const callback = setup_vars(vars, calc, input)
	const history = document.getElementById('history')

	setup_history(history, input)
	setup_input(input, calc, err, callback)

	/**
	 * @param title_html {string}
	 * @param contents {HTMLElement | string}
	 */
	show_dialog = function(title_html, contents) {
		const container = dialog_template.cloneNode(true)
		const dialog = container.lastElementChild

		dialog.firstElementChild.innerHTML = title_html

		if (typeof contents === 'string') {
			dialog.firstElementChild.nextElementSibling.innerText = contents
		} else {
			dialog.insertBefore(contents, dialog.lastElementChild)
			dialog.removeChild(dialog.firstElementChild.nextElementSibling)
		}

		dialogs++

		dialog.lastElementChild.addEventListener('click', () => {
			container.remove()

			if (--dialogs === 0) {
				input.focus()
			}
		})

		document.body.appendChild(container)
		dialog.lastElementChild.focus()
	}

	if ((/** @type {string[]} */ [... document.firstElementChild.classList]).findIndex(x => x.startsWith('gr__')) > -1) {
		show_dialog('Please disable Grammarly',
		            'This calculator won\'t work correctly with Grammarly' +
			            ' aggressively spell-checking the input field and' +
			            ' screwing up the contents. Please consider disabling' +
			            ' Grammarly for this site.')
	} else {
		input.focus()
	}
}

/**
 * @param elem {HTMLElement}
 * @param input {HTMLElement}
 */
function setup_history(elem, input) {
	const group_template = elem.removeChild(elem.firstElementChild)
	group_template.classList.add('flex')
	group_template.classList.remove('hidden')
	const entry_template = elem.removeChild(elem.firstElementChild)
	entry_template.classList.add('flex')
	entry_template.classList.remove('hidden')
	const button_template = entry_template.firstElementChild.removeChild(
		entry_template.firstElementChild.lastElementChild)
	button_template.classList.remove('hidden')
	const input_template = input.parentElement.cloneNode(true)
	input_template.lastElementChild.removeAttribute('contenteditable')
	input_template.lastElementChild.style.removeProperty('max-height')
	input_template.lastElementChild.style.removeProperty('user-modify')
	input_template.lastElementChild.style.removeProperty('-webkit-user-modify')
	input_template.lastElementChild.style.removeProperty('-moz-user-modify')

	elem.firstElementChild.lastElementChild.addEventListener('click', function() {
		history = []
		current_history = ['']
		history_index = 0
	})

	token_lookup[Module.TokenType.UNKNOWN.value] = 'UNKNOWN'
	token_lookup[Module.TokenType.NUMBER.value] = 'NUMBER'
	token_lookup[Module.TokenType.OPERATOR.value] = 'OPERATOR'
	token_lookup[Module.TokenType.UOPERATOR.value] = 'UOPERATOR'
	token_lookup[Module.TokenType.OPAREN.value] = 'OPAREN'
	token_lookup[Module.TokenType.CPAREN.value] = 'CPAREN'
	token_lookup[Module.TokenType.FOPAREN.value] = 'FOPAREN'
	token_lookup[Module.TokenType.FCPAREN.value] = 'FCPAREN'
	token_lookup[Module.TokenType.VARIABLE.value] = 'VARIABLE'
	token_lookup[Module.TokenType.FUNCTION.value] = 'FUNCTION'
	token_lookup[Module.TokenType.COMMA.value] = 'COMMA'

	function token_debug(token, tag_name = 'div') {
		const entry = document.createElement(tag_name)
		entry.classList.add('whitespace-pre')
		const type = entry.appendChild(document.createElement('span'))
		type.classList.add('text-grey-dark', 'inline-block')
		type.innerText = '(' + token_lookup[token.type] + ')'
		type.style.setProperty('width', '12ch')
		const content = entry.appendChild(document.createElement('span'))

		if (token_colors[token.type] !== '') {
			content.classList.add(token_colors[token.type])
		}

		content.innerText = token.data

		return entry
	}

	/**
	 * @param tokens {{type: number, data: string, pos: number}[]}
	 */
	function tokens_debug(tokens) {
		const div = document.createElement('div')
		const header = div.appendChild(document.createElement('strong'))
		header.classList.add('block', 'text-xs', 'mb-2')
		header.innerText = 'Tokens'
		const container = div.appendChild(document.createElement('div'))
		container.classList.add('overflow-auto')

		for (const token of tokens) {
			container.appendChild(token_debug(token)).classList.add('leading-normal')
		}

		return div
	}

	/**
	 * @param header {HTMLElement}
	 * @returns {HTMLElement}
	 */
	function new_button(header) {
		return /** @type {HTMLElement} */ header.appendChild(button_template.cloneNode(true))
	}

	/**
	 * @param ast {{type: number, token: Object, children: Array}}
	 * @param nested {boolean}
	 */
	function ast_debug(ast, nested = false) {
		const div = document.createElement('div')
		div.classList.add(nested ? 'overflow-visible' : 'overflow-auto', 'whitespace-pre')

		if (!nested) {
			const header = div.appendChild(document.createElement('div'))
			header.classList.add('flex', 'overflow-visible', 'mb-2', 'items-center')

			const header_text = header.appendChild(document.createElement('strong'))
			header_text.classList.add('block', 'text-xs')
			header_text.innerText = 'AST'

			header.appendChild(document.createElement('div')).classList.add('flex-grow')

			const expand_btn = header.appendChild(button_template.cloneNode(true))
			expand_btn.innerText = 'Expand all'
			expand_btn.classList.add('bg-blue-darker')
			expand_btn.classList.remove('bg-blue-darkest')

			let to_expand = true
			/**
			 * @param elem_to_expand {HTMLElement}
			 */
			let expand = function(elem_to_expand) {
				if (elem_to_expand.tagName.toLowerCase() === 'details') {
					if ((/** @type {HTMLDetailsElement} */ elem_to_expand).open !== to_expand) {
						elem_to_expand.firstElementChild.click()
					}
				}

				/**
				 * @type {HTMLElement}
				 */
				let child = /** @type {HTMLElement} */ elem_to_expand.firstElementChild

				while (child !== null) {
					expand(child)
					child = /** @type {HTMLElement} */ child.nextElementSibling
				}
			}

			expand_btn.addEventListener('click', () => {
				expand(div)
				to_expand = !to_expand
				expand_btn.innerText = to_expand ? 'Expand all' : 'Collapse all'
			})
		}

		if (ast.children.length > 0) {
			const details = div.appendChild(document.createElement('details'))

			if (!nested) {
				details.classList.add('overflow-auto')
			}

			details
				.appendChild(document.createElement('summary'))
				.appendChild(token_debug(ast.token, 'span'))
				.classList.add('leading-normal')
			const children = details.appendChild(document.createElement('div'))
			children.classList.add('ml-4')

			for (const child of ast.children) {
				children.appendChild(ast_debug(child, true)).classList.add('leading-normal')
			}
		} else {
			div
				.appendChild(token_debug(ast.token))
				.style.setProperty('margin-left', '1.06em')
		}

		return div
	}

	/**
	 * @param results {{input          : string,
	 *                  input_text     : string,
	 *                  tokens         : Array,
	 *                  ast            : Object,
	 *                  result         : HTMLElement,
	 *                  result_text    : string,
	 *                  console_output : string}}
	 */
	function create_results_screen(results) {
		const container = document.createElement('div')
		container.classList.add('m-2')

		const input = container.appendChild(input_template.cloneNode(true))
		input.lastElementChild.innerHTML = results.input

		const tokens_debug_elem = tokens_debug(results.tokens)
		tokens_debug_elem.classList.add('m-2')
		tokens_debug_elem.style.setProperty('width', '20rem')
		container.appendChild(tokens_debug_elem)

		if (typeof results.ast !== 'undefined') {
			const ast_debug_elem = ast_debug(results.ast)
			ast_debug_elem.classList.add('m-2')
			ast_debug_elem.style.setProperty('width', '20rem')
			container.appendChild(ast_debug_elem)
		}

		if (results.console_output !== '') {
			const terminal_output = container.appendChild(document.createElement('div'))
			const header = terminal_output.appendChild(document.createElement('strong'))
			header.classList.add('block', 'text-xs')
			header.innerText = 'Console output'

			const content = terminal_output.appendChild(document.createElement('pre'))
			content.classList.add('mt-2', 'leading-normal', 'overflow-auto')
			content.style.setProperty('width', '20rem')
			content.innerText = escape_str(results.console_output)
		}

		return container
	}

	/**
	 * @param header {HTMLElement}
	 * @param results {{input          : string,
	 *                  input_text     : string,
	 *                  tokens         : Array,
	 *                  ast            : Object,
	 *                  result         : HTMLElement,
	 *                  result_text    : string,
	 *                  console_output : string}}
	 */
	function create_debug_btn(header, results) {
		const button = new_button(header)
		button.innerText = 'Details'

		button.addEventListener('click', function() {
			show_dialog('Details', create_results_screen(results))
		})

		return button
	}

	new_entry_group = function() {
		elem.appendChild(group_template.cloneNode(true))
		elem.scrollTop = elem.scrollHeight - elem.clientHeight

		return elem.lastElementChild
	}

	/**
	 * @param results {{input          : string,
	 *                  input_text     : string,
	 *                  tokens         : Array,
	 *                  ast            : Object,
	 *                  result         : HTMLElement,
	 *                  result_text    : string,
	 *                  console_output : string}}
	 */
	add_to_history = function(results) {
		const entry = elem.lastElementChild.appendChild(entry_template.cloneNode(true))
		const header = entry.firstElementChild
		const debug_btn = create_debug_btn(header, results)
		const remove_btn = new_button(header)
		remove_btn.innerText = 'Remove'
		remove_btn.addEventListener('click', remove)

		const input_ = entry.appendChild(input_template.cloneNode(true))
		input_.lastElementChild.innerHTML = results.input

		const output = entry.appendChild(input_template.cloneNode(true))
		output.classList.add('-mt-2')
		output.firstElementChild.innerText = '='
		output.lastElementChild.appendChild(results.result)

		elem.firstElementChild.lastElementChild.addEventListener('click', remove)
		elem.scrollTop = elem.scrollHeight - elem.clientHeight

		function remove() {
			const parent = entry.parentElement
			entry.remove()

			if (parent !== null && parent.childElementCount === 0) {
				parent.remove()
			}

			input.focus()
		}

		return entry
	}
}

/**
 * @param elem {HTMLElement}
 * @param calc
 * @param input {HTMLElement}
 */
function setup_vars(elem, calc, input) {
	/**
	 * @type {[string, string, HTMLElement][]}
	 */
	const elems = []
	const template = elem.removeChild(elem.firstElementChild)
	const var_ = /** @type {HTMLElement} */ new_item('_', '')

	/**
	 * Copies `val` to the clipboard. Only works inside user input events.
	 *
	 * @param val {string} What to copy
	 */
	function copy_to_clipboard(val) {
		const temp = document.createElement('input')
		temp.style.opacity = '0'
		document.body.appendChild(temp)
		temp.value = val
		temp.select()
		document.execCommand('copy')
		temp.remove()
	}

	/**
	 * @param val {string}
	 * @returns {HTMLElement}
	 */
	highlight_val = function(val) {
		const container = document.createElement('span')
		container.innerHTML = val
			.replace(/[.,\/]/g, '<span class="text-grey-dark">$&</span>')
			.replace(/\(([^)]+)\)/, '<span class="border-t border-white">$1</span>')
		container.setAttribute('title', 'Click to copy value')
		container.addEventListener('click', function(e) {
			copy_to_clipboard(val)

			e.preventDefault()
			return false
		})

		return container
	}

	/**
	 * @param item {HTMLElement}
	 * @param key {string}
	 * @param val {string}
	 */
	function change_item(item, key, val) {
		item.firstElementChild.innerText = key
		item.removeChild(item.lastElementChild)

		if (val === '') {
			item.appendChild(template.lastElementChild.cloneNode(true))
		} else {
			const highlighted_val = highlight_val(val)

			item.appendChild(highlighted_val)
		}
	}

	/**
	 * @param variable {HTMLElement}
	 */
	function make_var_clickable(variable) {
		variable.addEventListener('mousedown', function(e) {
			if (document.activeElement !== input) {
				return false
			}

			const {prefix, suffix} = get_sections(input)

			const var_name = variable.innerText
			input.innerText = prefix + var_name + suffix
			rehighlight()

			set_sel_range(input,
			              prefix.length + var_name.length,
			              prefix.length + var_name.length)

			e.preventDefault()

			return false
		})
	}

	function new_item(key, val) {
		const item = /** @type {HTMLElement} */ elem.appendChild(template.cloneNode(true))
		make_var_clickable(/** @type {HTMLElement} */ item.firstElementChild)
		change_item(item, key, val)

		return item
	}

	return function() {
		/**
		 * @type {Object<string, string>}
		 */
		const vars = calc.get_vars()

		if (vars.hasOwnProperty('_')) {
			change_item(var_, '_', vars._)
		} else {
			change_item(var_, '_', '')
		}

		const keys = Object.keys(vars)

		for (let key of keys) {
			if (key === '_') continue

			const val = vars[key]
			const found = elems.find(x => x[0] === key)

			if (typeof found === 'undefined') {
				elems.push([
					           key,
					           val,
					           new_item(key, val)
				           ])
			} else if (found[1] !== val) {
				change_item(found[2], key, val)
				found[1] = val
			}
		}

		for (let i = 0; i < elems.length; i++) {
			const item = elems[i]

			if (!vars.hasOwnProperty(item[0])) {
				if (item[0] === '_') {
					change_item(var_, '_', '')
				} else {
					elem.removeChild(elems.splice(i, 1)[0][2])
					i--
				}
			}
		}
	}
}

/**
 * @param input {HTMLElement}
 * @param calc
 * @param err {HTMLElement}
 * @param update_vars {Function}
 */
function setup_input(input, calc, err, update_vars) {
	let errored = false

	document.body.addEventListener('keydown', function(e) {
		if (dialogs === 0 && !e.ctrlKey && !e.metaKey) {
			input.focus()
		}
	})

	document.body.addEventListener('paste', function() {
		if (dialogs === 0) {
			input.focus()
		}
	})

	input.parentNode.parentNode.addEventListener('click', function() {
		input.focus()

		return false
	})

	/**
	 * Escapes a string to prevent HTML injection issues.
	 *
	 * <hello&world> -> &lt;hello&amp;world&gt;
	 *
	 * @param str {string}
	 * @returns {string}
	 */
	escape_str = function(str) {
		return str
			.replace(/&/g, '&amp;')
			.replace(/>/g, '&gt;')
			.replace(/</g, '&lt;')
	}

	/**
	 * Splits a string by tokens. The first value returned is the split strings,
	 * and the second is the subset of `tokens` used to produce it. Some tokens
	 * are skipped, namely:
	 *
	 * - Implied multiplication (which do get counted as tokens) - those are
	 *   skipped
	 * - Tokens outside the range [offset, text.length + offset]
	 *
	 * @param text {string}
	 * @param tokens {Array}
	 * @param offset How many characters `text` is from the start of the string
	 * that was tokenized. This is effectively how much to subtract from each
	 * token's position to get the position in `text`.
	 * @returns {[string[], Array]}
	 */
	function split_by_token(text, tokens, offset = 0) {
		const split = []
		const tokens2 = []
		let last = offset

		tokens.forEach((token, i) => {
			if (token.pos >= last &&
				(i === tokens.length - 1 || token.pos < tokens[i + 1].pos) &&
				token.pos - offset < text.length) {
				split.push(text.substring(last - offset, token.pos - offset))
				split.push(text.substr(token.pos - offset, token.data.length))
				last = token.pos + token.data.length
				tokens2.push(token)
			}
		})

		split.push(text.substr(last - offset))

		return [
			split,
			tokens2
		]
	}

	/**
	 * @param text {string}
	 * @param _tokens {Array}
	 * @param offset {Number}
	 * @returns {string}
	 */
	function highlight_tokens(text, _tokens, offset = 0) {
		const [split, tokens] = split_by_token(text, _tokens, offset)

		tokens.forEach((token, i) => {
			const split_i = i * 2 + 1

			split[split_i - 1] = escape_str(split[split_i - 1])

			if (token_colors[token.type] !== '') {
				split[split_i] = '<span class="' + token_colors[token.type] + '">' +
					escape_str(split[split_i]) + '</span>'
			} else {
				split[split_i] = escape_str(split[split_i])
			}
		})

		split[split.length - 1] = escape_str(split[split.length - 1])

		return split.join('')
	}

	/**
	 * @param vec
	 * @returns {Array}
	 */
	function vector_to_array(vec) {
		const arr = []
		const size = vec.size()

		for (let i = 0; i < size; i++) {
			arr.push(vec.get(i))
		}

		return arr
	}

	function ast_to_object(ast) {
		return {
			type     : ast.type,
			token    : ast.token,
			children : vector_to_array(ast.children).map(ast_to_object)
		}
	}

	/**
	 * @param err {HTMLElement}
	 * @param msg {string}
	 */
	function set_err(err, msg) {
		errored = msg !== ''

		if (msg !== '') {
			err.innerText = msg
			err.classList.remove('hidden')
			err.parentNode.classList.add('bg-red')
			err.parentNode.classList.remove('bg-blue-darker')
		} else {
			err.classList.add('hidden')
			err.parentNode.classList.add('bg-blue-darker')
			err.parentNode.classList.remove('bg-red')
		}
	}

	/**
	 * @param results {{input: string, input_text: string, tokens: Array, ast: Object, result: HTMLElement, result_text: string}}
	 * @param group {HTMLElement}
	 */
	function on_execute(results, group) {
		con_active = false
		results.console_output = con_buffer
		con_buffer = ''

		if (typeof group === 'undefined') {
			add_to_history(results)
		} else {
			group.appendChild(add_to_history(results))
		}
	}

	/**
	 * @param src {string} Input string
	 * @param highlighted_src {string}
	 * @param calc
	 */
	function execute_command(src, highlighted_src, calc) {
		con_active = true

		try {
			const tokens = calc.tokenize_command(src)
			const result = calc.execute_command(tokens)

			update_vars()
			on_execute(
				{
					input       : highlighted_src,
					input_text  : src,
					tokens      : vector_to_array(tokens),
					ast         : undefined,
					result      : document.createTextNode(result),
					result_text : result
				}
			)
		} catch (up) {
			con_active = false

			throw up
		}
	}

	/**
	 * @param src {string} Input string
	 * @param highlighted_src {string}
	 * @param calc
	 */
	function execute_expression(src, highlighted_src, calc) {
		con_active = true

		try {
			const tokens = calc.tokenize(src)
			const ast = calc.get_ast(tokens)
			const result = calc.execute_ast(ast)

			update_vars()
			on_execute(
				{
					input       : highlighted_src,
					input_text  : src,
					tokens      : vector_to_array(tokens),
					ast         : ast_to_object(ast),
					result      : highlight_val(result),
					result_text : result
				}
			)
		} catch (up) {
			con_active = false

			throw up
		}
	}

	/**
	 * @param text {string}
	 * @param calc
	 * @param group {HTMLElement}
	 * @param execute {boolean} Whether to actually execute, if it can
	 * @returns {string}
	 */
	function highlight_subexp(text, calc, execute = false) {
		const command = text.charAt(0) === ':'
		let tokens,
		    ast,
		    got_tokens

		try {
			if (text.trim() !== '') {
				let tokens_v = command ? calc.tokenize_command(text) : calc.tokenize(text)
				tokens = vector_to_array(tokens_v)
				got_tokens = true

				if (command) {
					if (execute) {
						execute_command(text, highlight_subexp(text, calc), calc)
					} else {
						calc.validate_command(tokens_v)
					}
				} else {
					ast = calc.get_ast(tokens_v)

					if (execute) {
						execute_expression(text, highlight_subexp(text, calc), calc)
					} else {
						calc.valid_ast(ast)
					}
				}
			} else {
				return escape_str(text)
			}
		} catch (e) {
			set_err(err, e.get_msg())

			if (!command && !got_tokens) {
				tokens = vector_to_array(calc.last_tokens)
			}

			let token = e.get_token()

			if (!got_tokens) {
				return highlight_tokens(text.substr(0, token.pos), tokens) +
					'<span class="text-red underline">' +
					escape_str(text.substr(token.pos, token.data.length)) +
					'</span><span class="opacity-25">' +
					escape_str(text.substr(token.pos + token.data.length)) +
					'</span>'
			} else {
				return highlight_tokens(text.substr(0, token.pos), tokens) +
					'<span class="text-red underline">' +
					escape_str(text.substr(token.pos, token.data.length)) +
					'</span>' +
					highlight_tokens(text.substr(token.pos + token.data.length),
					                 tokens, token.pos + token.data.length)
			}
		}

		return highlight_tokens(text, tokens)
	}

	/**
	 * @param text {string}
	 * @param calc
	 * @param err {HTMLElement}
	 * @param execute {boolean}
	 */
	highlight_exp = function(text, calc, err, execute) {
		set_err(err, '')

		let group

		if (execute) {
			group = new_entry_group()
		}

		let split

		try {
			split = text.split(';')
			            .map(subexp => highlight_subexp(subexp, calc, execute))
		} catch (up) {
			if (typeof group !== 'undefined' && group.childElementCount === 0) {
				group.remove()
			}

			throw up
		}

		if (typeof group !== 'undefined' && group.childElementCount === 0) {
			group.remove()
		}

		return split.join('<span class="text-yellow">;</span>')
	}

	/**
	 * @param node {Node}
	 * @returns {Text[]}
	 */
	function get_text_nodes(node) {
		/**
		 * @type {Text[]}
		 */
		let textNodes = []

		if (node.nodeType === 3) {
			textNodes.push(/** @type {Text} */ node)
		} else {
			let children = node.childNodes

			for (let i = 0, len = children.length; i < len; i++) {
				textNodes.push(... get_text_nodes(children[i]))
			}
		}

		return textNodes
	}

	/**
	 * Sets the region of text selected by the user to the range between [start,
	 * end]. If `start === end`, no text will be selected.
	 *
	 * @param elem {HTMLElement}
	 * @param start Index
	 * @param end Index
	 */
	set_sel_range = function(elem, start, end) {
		if (document.createRange && window.getSelection) {
			let range = document.createRange()
			range.selectNodeContents(elem)

			let textNodes  = get_text_nodes(elem),
			    charCount  = 0,
			    foundStart = false

			for (let i = 0, textNode; textNode = textNodes[i]; i++) {
				let endCharCount = charCount + textNode.length

				if (!foundStart && start >= charCount && (start < endCharCount ||
					(start === endCharCount && i <= textNodes.length))) {
					range.setStart(textNode, start - charCount)
					foundStart = true
				}

				if (foundStart && end <= endCharCount) {
					range.setEnd(textNode, end - charCount)

					break
				}
				charCount = endCharCount
			}

			let sel = window.getSelection()
			sel.removeAllRanges()
			sel.addRange(range)
		} else if (typeof document.selection !== 'undefined' && document.body.createTextRange) {
			let textRange = document.body.createTextRange()
			textRange.moveToElementText(elem)
			textRange.collapse(true)
			textRange.moveEnd('character', end)
			textRange.moveStart('character', start)
			textRange.select()
		}
	}

	/**
	 * @param node {HTMLElement}
	 * @returns {{prefix : string, selected : string, suffix : string}}
	 */
	get_sections = function(node) {
		let prefix   = '',
		    selected = '',
		    suffix   = ''

		if (typeof window.getSelection !== 'undefined') {
			let selection = window.getSelection()
			selected = selection.toString()

			let range

			if (selection.rangeCount) {
				range = selection.getRangeAt(0)
			} else {
				range = document.createRange()
				range.collapse(true)
			}

			let tempRange = document.createRange()
			tempRange.selectNodeContents(node)

			tempRange.setEnd(range.startContainer, range.startOffset)
			prefix = tempRange.toString()
			tempRange.selectNodeContents(node)

			tempRange.setStart(range.endContainer, range.endOffset)
			suffix = tempRange.toString()
			tempRange.detach()
		} else if (document.selection && document.selection.type !== 'Control') {
			let range = document.selection.createRange()
			let tempRange = document.body.createTextRange()

			selected = tempRange.text

			tempRange.moveToElementText(node)
			tempRange.setEndPoint('EndToStart', range)
			prefix = tempRange.text

			tempRange.moveToElementText(node)
			tempRange.setEndPoint('StartToEnd', range)
			suffix = tempRange.text
		}

		return {
			prefix   : prefix,
			selected : selected,
			suffix   : suffix
		}
	}

	/**
	 * @param execute Whether to execute the AST as well
	 * @returns The innerText of the input field
	 */
	rehighlight = function(execute = false) {
		const {prefix, selected} = get_sections(input)
		const text = input.innerText

		input.innerHTML = highlight_exp(text, calc, err, execute)
		set_sel_range(input, prefix.length, (prefix + selected).length)

		return text
	}

	input.addEventListener('input', () => rehighlight())

	input.addEventListener('keydown', function(e) {
		if (!e.shiftKey) {
			switch (e.keyCode) {
				case 13: // enter
					if (!errored) {
						const text = rehighlight(true)

						if (!errored) {
							input.innerText = ''
							rehighlight()

							history.unshift(text)

							current_history = [
								'',
								... history
							]

							history_index = 0
						}
					}

					e.preventDefault()
					break
				case 38: // up arrow
					if (history_index < current_history.length - 1) {
						current_history[history_index] = rehighlight()
						input.innerText = current_history[++history_index]

						rehighlight()
					}

					e.preventDefault()
					break
				case 40: // down arrow
					if (history_index > 0) {
						current_history[history_index] = rehighlight()
						input.innerText = current_history[--history_index]

						rehighlight()
					}
			}
		}

		return true
	})
}