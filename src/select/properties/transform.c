/*
 * This file is part of LibCSS
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "select/propset.h"
#include "select/propget.h"
#include "utils/utils.h"

#include "select/properties/properties.h"
#include "select/properties/helpers.h"

css_error css__cascade_transform(uint32_t opv, css_style *style,
		css_select_state *state)
{
	uint16_t value = CSS_TRANSFORM_INHERIT;
	css_matrix matrix = {{1024,0,0,1024,0,0}};

	if (hasFlagValue(opv) == false) {
		switch (getValue(opv)) {
		case TRANSFORM_NONE:
			value = CSS_TRANSFORM_NONE;
			break;
		case TRANSFORM_SET:
			value = CSS_TRANSFORM_SET;
			matrix = *((css_matrix *) style->bytecode);
			advance_bytecode(style, sizeof(matrix));
			break;
		}
	}

	if (css__outranks_existing(getOpcode(opv), isImportant(opv), state,
			getFlagValue(opv))) {
		return set_transform(state->computed, value, &matrix);
	}

	return CSS_OK;
}

css_error css__set_transform_from_hint(const css_hint *hint,
		css_computed_style *style)
{
	return set_transform(style, hint->status, hint->data.transform);
}

css_error css__initial_transform(css_select_state *state)
{
	css_matrix matrix = {{1024,0,0,1024,0,0}};
	return set_transform(state->computed,
			CSS_TRANSFORM_NONE, &matrix);
}

css_error css__copy_transform(
		const css_computed_style *from,
		css_computed_style *to)
{
	css_matrix matrix;
	uint8_t type = get_transform(from, &matrix);

	if (from == to) {
		return CSS_OK;
	}

	return set_transform(to, type, &matrix);
}

css_error css__compose_transform(const css_computed_style *parent,
		const css_computed_style *child,
		css_computed_style *result)
{
	css_matrix matrix;
	uint8_t type = get_transform(child, &matrix);

	return css__copy_transform(
			type == CSS_TRANSFORM_INHERIT ? parent : child,
			result);
}
