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

css_error css__cascade_fill(uint32_t opv, css_style *style,
		css_select_state *state)
{
	return css__cascade_paint(opv, style, state, set_fill);
}

css_error css__set_fill_from_hint(const css_hint *hint,
		css_computed_style *style)
{
	if (hint->status == CSS_PAINT_URI) {
		return set_fill(style, hint->status, 0, hint->data.string);
	}
	return set_fill(style, hint->status, hint->data.color, NULL);
}

css_error css__initial_fill(css_select_state *state)
{
	return set_fill(state->computed,
			CSS_PAINT_COLOR, 0xff000000, NULL);
}

css_error css__copy_fill(
		const css_computed_style *from,
		css_computed_style *to)
{
	css_color color;
	lwc_string *uri;
	uint8_t type = get_fill(from, &color, &uri);

	if (from == to) {
		return CSS_OK;
	}

	return set_fill(to, type, color, uri);
}

css_error css__compose_fill(const css_computed_style *parent,
		const css_computed_style *child,
		css_computed_style *result)
{
	css_color color;
	lwc_string *uri;
	uint8_t type = get_fill(child, &color, &uri);

	return css__copy_fill(
			type == CSS_PAINT_INHERIT ? parent : child,
			result);
}
