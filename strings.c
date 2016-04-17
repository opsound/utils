#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <regex.h>

#define STR \
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\n" \
	"incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis\n" \
	"nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo\n" \
	"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum\n" \
	"dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n" \
	"sunt in culpa qui officia deserunt mollit anim id est laborum.\n"

char *lorem = STR;

// Removes whitespace in-place from s
char *strip(char *s)
{
	char *a, *b;
	a = b = s;
	while (*b != '\0') {
		if (isspace(*b)) b++;
		else *a++ = *b++;
	}
	*a = '\0';
	return s;
}

void freestrings(char **s)
{
	for (size_t i = 0; s[i]; i++) {
		free(s[i]);
	}
	free(s);
}

// Returns pos null-terminated array of strings from s delimited by any of the
// characters in delim
char **split(const char *s, const char *delim, size_t *nstrings)
{
	char *token, *input, *tofree;

	tofree = input = strdup(s);

	size_t n, n_empty;
	n = n_empty = 32;

	char **strings, **curstring;
	strings = curstring = calloc(n, sizeof(*strings));

	while ((token = strsep(&input, delim)) != NULL) {
		if (!*token) continue; // skip appending empty strings

		*curstring++ = strdup(token);
		n_empty--;
		if (!n_empty) {
			n_empty = n;
			n *= 2;

			strings = realloc(strings, n * sizeof(*strings));
			if (!strings) return NULL;

			curstring = &strings[n_empty];
		}
	}
	*curstring = NULL;
	n_empty--;
	n -= n_empty;

	free(tofree);

	*nstrings = n - 1; // don't include terminating null pointer
	return strings;
}

char *join(const char * const *strings, const char *separator)
{
	size_t n = 32;
	size_t n_empty = n - 1; // null terminator

	char *joined = calloc(n, sizeof(*joined));
	if (!joined) return NULL;

	for (size_t i = 0; strings[i]; i++) {
		if (!*strings[i]) continue; // skip joining empty strings

		size_t catlen = strlen(strings[i]) + strlen(separator);
		while (catlen > n_empty) {
			n_empty += n;
			n *= 2;
			joined = realloc(joined, n);
			if (!joined) return NULL;
		}
		strcat(joined, strings[i]);
		strcat(joined, separator);
		n_empty -= catlen;
	}

	n -= n_empty;
	joined = realloc(joined, n);
	if (!joined) return NULL;

	return joined;
}

char *replace(const char *string, const char *pat, const char *rep)
{
	size_t pat_len = strlen(pat);
	size_t rep_len = strlen(rep);

	char *match;
	const char *search = string;
	size_t n_matches = 0;

	while ((match = strstr(search, pat)) != NULL) {
		n_matches++;
		search = &match[pat_len];
	}

	size_t output_len = strlen(string) + n_matches*(rep_len - pat_len) + 1;
	char *output = calloc(output_len, 1);
	char *pos = output;

	search = string;

	while ((match = strstr(search, pat)) != NULL) {
		ptrdiff_t len_before_match = match - search;
		memcpy(pos, search, len_before_match);
		pos += len_before_match;

		memcpy(pos, rep, rep_len);
		pos += rep_len;

		search = &match[pat_len];
	}

	strcpy(pos, search);

	return output;
}

int main(int argc, char *argv[])
{
	size_t n_strings;
	char **strings = split(lorem, "\n", &n_strings);
	for (size_t i = 0; strings[i]; i++) {
		/* printf("%s\n", strings[i]); */
	}

	freestrings(strings);

	char *r1 = replace(lorem, " ", "");
	printf("%s\n", r1);
	free(r1);

	return 0;
}
