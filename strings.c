#include <ctype.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Returns a null-terminated array of strings from s delimited by any of the
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

char *replace_reg(const char *string, const char *pat, const char *rep)
{
	regmatch_t match;
	regex_t reg;

	const char *search = string;
	size_t output_len = 0;

	if (regcomp(&reg, pat, REG_EXTENDED) != 0) return NULL;

	while (regexec(&reg, search, 1, &match, 0) == 0) {
		output_len += match.rm_so;
		output_len += strlen(rep);

		search = &search[match.rm_eo];
	}
	output_len += strlen(search);

	char *output = calloc(output_len + 1, 1);
	char *pos = output;
	search = string;

	while (regexec(&reg, search, 1, &match, 0) == 0) {
		memcpy(pos, search, match.rm_so);
		pos += match.rm_so;
		memcpy(pos, rep, strlen(rep));
		pos += strlen(rep);

		search = &search[match.rm_eo];
	}
	strcpy(pos, search);

	regfree(&reg);

	return output;
}

bool contains_reg(const char *string, const char *pat)
{
	bool ret = false;
	regex_t reg;
	regmatch_t match;

	if (regcomp(&reg, pat, REG_EXTENDED) != 0) return false;

	if (regexec(&reg, string, 1, &match, 0) == 0)
		ret = true;

	regfree(&reg);

	return ret;
}

char *read_all(FILE *fp, size_t *n)
{
	size_t len = 0;
	size_t alloc_len = 32;
	size_t vacant_len = alloc_len;

	char *buf = calloc(alloc_len, 1);
	if (!buf) return NULL;

	size_t n_read = 0;
	while ((n_read = fread(&buf[len], 1, vacant_len, fp))) {
		vacant_len -= n_read;
		len += n_read;

		if (vacant_len == 0) {
			vacant_len = alloc_len;
			alloc_len *= 2;

			buf = realloc(buf, alloc_len);
			if (!buf) return NULL;
		}
	}

	*n = len;
	return buf;
}

int main(int argc, char *argv[])
{
	size_t n_strings;

	FILE *fp = fopen("lorem.txt", "r");
	size_t len;
	char *lorem = read_all(fp, &len);

	char **strings = split(lorem, "\n", &n_strings);
	for (size_t i = 0; strings[i]; i++) {
		printf("%s\n", strings[i]);

		if (contains_reg(strings[i], "[a-z]{3}")) {
			char *replaced = replace_reg(strings[i], "[a-z]*", "XXX");
			printf("%s\n\n", replaced);
			free(replaced);
		}
	}

	freestrings(strings);

	char *r1 = replace(lorem, " ", "");
	printf("%s\n", r1);
	free(r1);

	char *r2 = replace_reg(lorem, "[a-z]\\{3\\}", "XXX");
	printf("%s\n", r2);
	free(r2);

	free(lorem);

	return 0;
}
