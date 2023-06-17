/*
 * This file is part of LibCSS.
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Parse margin shorthand
 *
 * \param c	  Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx	  Pointer to vector iteration context
 * \param result  Pointer to location to receive resulting style
 * \return CSS_OK on success,
 *	   CSS_NOMEM on memory exhaustion,
 *	   CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *		   If the input is invalid, then \a *ctx remains unchanged.
 */
css_error css__parse_margin(css_language *c,
		const parserutils_vector *vector, int32_t *ctx,
		css_style *result)
{
	int32_t orig_ctx = *ctx;
	int prev_ctx;
	const css_token *token;
	uint16_t side_val[4];
	css_fixed side_length[4];
	uint32_t side_unit[4];
	uint32_t side_count = 0;
	bool match;
	css_error error;
	enum flag_value flag_value;

	/* Firstly, handle inherit */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	flag_value = get_css_flag_value(c, token);

	if (flag_value != FLAG_VALUE__NONE) {
		error = css_stylesheet_style_flag_value(result, flag_value,
				CSS_PROP_MARGIN_TOP);
		if (error != CSS_OK)
			return error;

		error = css_stylesheet_style_flag_value(result, flag_value,
				CSS_PROP_MARGIN_RIGHT);
		if (error != CSS_OK)
			return error;

		error = css_stylesheet_style_flag_value(result, flag_value,
				CSS_PROP_MARGIN_BOTTOM);
		if (error != CSS_OK)
			return error;

		error = css_stylesheet_style_flag_value(result, flag_value,
				CSS_PROP_MARGIN_LEFT);
		if (error == CSS_OK)
			parserutils_vector_iterate(vector, ctx);

		return error;
	}

	/* Attempt to parse up to 4 widths */
	do {
		prev_ctx = *ctx;

		flag_value = get_css_flag_value(c, token);

		if ((token != NULL) && flag_value != FLAG_VALUE__NONE) {
			*ctx = orig_ctx;
			return CSS_INVALID;
		}

		if ((token->type == CSS_TOKEN_IDENT) && (lwc_string_caseless_isequal(token->idata, c->strings[AUTO], &match) == lwc_error_ok && match)) {
			side_val[side_count] =  MARGIN_AUTO;
			parserutils_vector_iterate(vector, ctx);
			error = CSS_OK;
		} else {
			side_val[side_count] = MARGIN_SET;

			error = css__parse_unit_specifier(c, vector, ctx, UNIT_PX, &side_length[side_count], &side_unit[side_count]);
			if (error == CSS_OK) {
				if (side_unit[side_count] & UNIT_ANGLE||
				    side_unit[side_count] & UNIT_TIME||
				    side_unit[side_count] & UNIT_FREQ) {
					*ctx = orig_ctx;
					return CSS_INVALID;
				}
			}
		}

		if (error == CSS_OK) {
			side_count++;

			consumeWhitespace(vector, ctx);

			token = parserutils_vector_peek(vector, *ctx);
		} else {
			/* Forcibly cause loop to exit */
			token = NULL;
		}
	} while ((*ctx != prev_ctx) && (token != NULL) && (side_count < 4));


#define SIDE_APPEND(OP,NUM)								\
	error = css__stylesheet_style_appendOPV(result, (OP), 0, side_val[(NUM)]);	\
	if (error != CSS_OK)								\
		break;									\
	if (side_val[(NUM)] == MARGIN_SET) {						\
		error = css__stylesheet_style_append(result, side_length[(NUM)]);	\
		if (error != CSS_OK)							\
			break;								\
		error = css__stylesheet_style_append(result, side_unit[(NUM)]);		\
		if (error != CSS_OK)							\
			break;								\
	}

	switch (side_count) {
	case 1:
		SIDE_APPEND(CSS_PROP_MARGIN_TOP, 0);
		SIDE_APPEND(CSS_PROP_MARGIN_RIGHT, 0);
		SIDE_APPEND(CSS_PROP_MARGIN_BOTTOM, 0);
		SIDE_APPEND(CSS_PROP_MARGIN_LEFT, 0);
		break;
	case 2:
		SIDE_APPEND(CSS_PROP_MARGIN_TOP, 0);
		SIDE_APPEND(CSS_PROP_MARGIN_RIGHT, 1);
		SIDE_APPEND(CSS_PROP_MARGIN_BOTTOM, 0);
		SIDE_APPEND(CSS_PROP_MARGIN_LEFT, 1);
		break;
	case 3:
		SIDE_APPEND(CSS_PROP_MARGIN_TOP, 0);
		SIDE_APPEND(CSS_PROP_MARGIN_RIGHT, 1);
		SIDE_APPEND(CSS_PROP_MARGIN_BOTTOM, 2);
		SIDE_APPEND(CSS_PROP_MARGIN_LEFT, 1);
		break;
	case 4:
		SIDE_APPEND(CSS_PROP_MARGIN_TOP, 0);
		SIDE_APPEND(CSS_PROP_MARGIN_RIGHT, 1);
		SIDE_APPEND(CSS_PROP_MARGIN_BOTTOM, 2);
		SIDE_APPEND(CSS_PROP_MARGIN_LEFT, 3);
		break;
	default:
		error = CSS_INVALID;
	}

	if (error != CSS_OK)
		*ctx = orig_ctx;

	return error;
}

