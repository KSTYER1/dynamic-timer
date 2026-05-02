/* Format-time token parser. */

#include "format-time.h"

#include <util/dstr.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct token {
	const char *tok;
	int len;
};

/* Order matters: longer tokens FIRST so we don't accidentally match %H inside %0H. */
static const struct token g_tokens[] = {
	{"%3t", 3}, {"%2t", 3},
	{"%0H", 3}, {"%0M", 3}, {"%0S", 3},
	{"%0h", 3}, {"%0m", 3}, {"%0s", 3},
	{"%t",  2},
	{"%H",  2}, {"%M",  2}, {"%S",  2},
	{"%h",  2}, {"%m",  2}, {"%s",  2},
	{"%d",  2},
};

static void replace_all(struct dstr *s, const char *needle, const char *value)
{
	size_t nlen = strlen(needle);
	if (nlen == 0)
		return;
	size_t i = 0;
	while (i + nlen <= s->len) {
		if (memcmp(s->array + i, needle, nlen) == 0) {
			dstr_remove(s, i, nlen);
			dstr_insert(s, i, value);
			i += strlen(value);
		} else {
			i++;
		}
	}
}

void format_time_ns(int64_t ns, const char *fmt, char *out, size_t out_size)
{
	if (!out || out_size == 0)
		return;
	out[0] = '\0';
	if (!fmt) {
		return;
	}

	if (ns < 0)
		ns = 0;

	int64_t ms = ns / 1000000;

	int64_t H_inf = ms / 3600000;             /* unbounded hours */
	int64_t M_inf = ms / 60000;               /* unbounded minutes */
	int64_t S_inf = ms / 1000;                /* unbounded seconds */
	int64_t h_mod = (ms / 3600000) % 24;
	int64_t m_mod = (ms / 60000) % 60;
	int64_t s_mod = (ms / 1000) % 60;
	int64_t d_val = ms / 86400000;
	int     dec   = (int)(ms % 1000);         /* 0..999 */

	char buf3[8];
	snprintf(buf3, sizeof(buf3), "%03d", dec);
	const char *thousandths = buf3;            /* "000".."999" */
	char buf2[4] = {buf3[0], buf3[1], 0, 0};   /* first 2 digits */
	const char *hundredths = buf2;
	char buf1[2] = {buf3[0], 0};
	const char *tenths = buf1;

	struct dstr s;
	dstr_init_copy(&s, fmt);

	char tmp[32];

	/* Order: longest first; inside groups, the leading-zero variants are longer than plain. */
	(void)g_tokens; /* keep table for documentation */

	snprintf(tmp, sizeof(tmp), "%s", thousandths);
	replace_all(&s, "%3t", tmp);

	snprintf(tmp, sizeof(tmp), "%s", hundredths);
	replace_all(&s, "%2t", tmp);

	snprintf(tmp, sizeof(tmp), "%02lld", (long long)H_inf);
	replace_all(&s, "%0H", tmp);

	snprintf(tmp, sizeof(tmp), "%02lld", (long long)M_inf);
	replace_all(&s, "%0M", tmp);

	snprintf(tmp, sizeof(tmp), "%02lld", (long long)S_inf);
	replace_all(&s, "%0S", tmp);

	snprintf(tmp, sizeof(tmp), "%02lld", (long long)h_mod);
	replace_all(&s, "%0h", tmp);

	snprintf(tmp, sizeof(tmp), "%02lld", (long long)m_mod);
	replace_all(&s, "%0m", tmp);

	snprintf(tmp, sizeof(tmp), "%02lld", (long long)s_mod);
	replace_all(&s, "%0s", tmp);

	snprintf(tmp, sizeof(tmp), "%s", tenths);
	replace_all(&s, "%t", tmp);

	snprintf(tmp, sizeof(tmp), "%lld", (long long)H_inf);
	replace_all(&s, "%H", tmp);

	snprintf(tmp, sizeof(tmp), "%lld", (long long)M_inf);
	replace_all(&s, "%M", tmp);

	snprintf(tmp, sizeof(tmp), "%lld", (long long)S_inf);
	replace_all(&s, "%S", tmp);

	snprintf(tmp, sizeof(tmp), "%lld", (long long)h_mod);
	replace_all(&s, "%h", tmp);

	snprintf(tmp, sizeof(tmp), "%lld", (long long)m_mod);
	replace_all(&s, "%m", tmp);

	snprintf(tmp, sizeof(tmp), "%lld", (long long)s_mod);
	replace_all(&s, "%s", tmp);

	snprintf(tmp, sizeof(tmp), "%lld", (long long)d_val);
	replace_all(&s, "%d", tmp);

	snprintf(out, out_size, "%s", s.array ? s.array : "");
	dstr_free(&s);
}
