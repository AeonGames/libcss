#include <inttypes.h>
#include <stdio.h>

#include <parserutils/parserutils.h>
#include <libcss/libcss.h>
#include "stylesheet.h"

#define ITERATIONS (1)
#define DUMP_CSS (1)

#if DUMP_CSS
#include "dump.h"
#endif

#include "testutils.h"

static void *myrealloc(void *ptr, size_t len, void *pw)
{
	UNUSED(pw);

	return realloc(ptr, len);
}

static css_error resolve_url(void *pw,
		const char *base, lwc_string *rel, lwc_string **abs)
{
	UNUSED(pw);
	UNUSED(base);

	/* About as useless as possible */
	*abs = lwc_string_ref(rel);

	return CSS_OK;
}

int main(int argc, char **argv)
{
	css_stylesheet *sheet;
	FILE *fp;
	size_t len, origlen;
#define CHUNK_SIZE (4096)
	uint8_t buf[CHUNK_SIZE];
	css_error error;
	int count;

	if (argc != 3) {
		printf("Usage: %s <aliases_file> <filename>\n", argv[0]);
		return 1;
	}

	/* Initialise library */
	assert(parserutils_initialise(argv[1], myrealloc, NULL) == PARSERUTILS_OK);
        
        assert(lwc_initialise(myrealloc, NULL, 0) == lwc_error_ok);
        
	for (count = 0; count < ITERATIONS; count++) {

		assert(css_stylesheet_create(CSS_LEVEL_21, "UTF-8", argv[2], 
				NULL, false, false, myrealloc, NULL, 
				resolve_url, NULL, &sheet) == CSS_OK);

		fp = fopen(argv[2], "rb");
		if (fp == NULL) {
			printf("Failed opening %s\n", argv[2]);
			return 1;
		}

		fseek(fp, 0, SEEK_END);
		origlen = len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		while (len >= CHUNK_SIZE) {
			size_t read = fread(buf, 1, CHUNK_SIZE, fp);
			assert(read == CHUNK_SIZE);

			error = css_stylesheet_append_data(sheet, buf, 
					CHUNK_SIZE);
			assert(error == CSS_OK || error == CSS_NEEDDATA);

			len -= CHUNK_SIZE;
		}

		if (len > 0) {
			size_t read = fread(buf, 1, len, fp);
			assert(read == len);

			error = css_stylesheet_append_data(sheet, buf, len);
			assert(error == CSS_OK || error == CSS_NEEDDATA);

			len = 0;
		}

		fclose(fp);

		error = css_stylesheet_data_done(sheet);
		assert(error == CSS_OK || error == CSS_IMPORTS_PENDING);

		while (error == CSS_IMPORTS_PENDING) {
			lwc_string *url;
			uint64_t media;

			error = css_stylesheet_next_pending_import(sheet,
					&url, &media);
			assert(error == CSS_OK || error == CSS_INVALID);

			if (error == CSS_OK) {
				css_stylesheet *import;
				char *buf = alloca(lwc_string_length(url) + 1);

				memcpy(buf, lwc_string_data(url), 
						lwc_string_length(url));
				buf[lwc_string_length(url)] = '\0';

				assert(css_stylesheet_create(CSS_LEVEL_21,
					"UTF-8", buf, NULL, false, false, 
					myrealloc, NULL, resolve_url, NULL,
					&import) == CSS_OK);

				assert(css_stylesheet_data_done(import) == 
					CSS_OK);

				assert(css_stylesheet_register_import(sheet,
					import) == CSS_OK);

				css_stylesheet_destroy(import);

				error = CSS_IMPORTS_PENDING;
			}
		}

#if DUMP_CSS
		{
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
			char *out;
			size_t outsize = max(16384, origlen * 8);
			size_t outlen = outsize;
			size_t written;
			out = malloc(outsize);
			assert(out != NULL);
			dump_sheet(sheet, out, &outlen);
			written = fwrite(out, 1, outsize - outlen, stdout);
			assert(written == outsize - outlen);
			free(out);
		}
#endif

		css_stylesheet_destroy(sheet);
	}

	assert(parserutils_finalise(myrealloc, NULL) == PARSERUTILS_OK);

	printf("PASS\n");
        
	return 0;
}

