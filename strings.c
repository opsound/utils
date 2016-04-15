#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *lorem =
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\n"
	"incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis\n"
	"nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo\n"
	"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum\n"
	"dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n"
	"sunt in culpa qui officia deserunt mollit anim id est laborum.\n";

// Removes whitespace in-place from s
char *strip(char *s) {
	char *a, *b;
	a = b = s;
	while (*b != '\0') {
		if (isspace(*b)) b++;
		else *a++ = *b++;
	}
	*a = '\0';
	return s;
}

// Returns a null-terminated array of strings from s delimited by any of the
// characters in delim
char **split(const char *s, const char *delim, size_t *nstrings) {
	char *token, *input, *tofree;

	tofree = input = strdup(s);

	size_t n, n_empty;
	n = n_empty = 1;

	char **strings, **curstring;
	strings = curstring = calloc(n, sizeof(*strings));

	while ((token = strsep(&input, delim)) != NULL) {
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
	n -= n_empty;

	free(tofree);

	*nstrings = n - 1; // don't include terminating null pointer
	return strings;
}

int main(int argc, char *argv[])
{
	size_t n_strings;
	char **strings = split(lorem, "\n", &n_strings);
	printf("%zu\n", n_strings);
	char *line;
	while ((line = *strings++)) {
		if (*line)
			printf("%s\n", strip(line));
	}

	return 0;
}
