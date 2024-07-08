#include <stdio.h>
#include "object.h"
#include "Parser/tokenizer.h"
#include "token.h"
#include "cpython/pythonrun.h"

#define DEBUG_TOKENIZER 0

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s PATH\n", argv[0]);
		return -1;
	}
	char *path = argv[1];
	FILE *fp = fopen(path, "r");
	assert(fp);

	#if DEBUG_TOKENIZER
	struct tok_state *tokenizer = PyTokenizer_FromFile(fp, NULL, NULL, NULL);
	int tok;
	while (1) {
		const char *start, *end;
		tok = tok_get(tokenizer, &start, &end);
		printf("tok is %d (%s): [%.*s]\n", tok, _get_token_name(tok), (int) (end - start), start);
		if (tok == ENDMARKER) {
			break;
		}
	}
	
	#endif
	pyrun_file(fp);

	fclose(fp);
	printf("bye\n");
	return 0;
}

#include "Parser/parser.c"
