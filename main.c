#include <stdio.h>
#include "Parser/tokenizer.h"
#include "token.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s PATH\n", argv[0]);
		return -1;
	}
	char *path = argv[1];
	FILE *fp = fopen(path, "r");
	assert(fp);

	struct tok_state *tokenizer = PyTokenizer_FromFile(fp, NULL, NULL, NULL);
	int tok;
	while (1) {
		tok = tok_get(tokenizer, NULL, NULL);
		printf("tok is %d (%s)\n", tok, _get_token_name(tok));
		if (tok == ENDMARKER) {
			break;
		}
	}
	
	fclose(fp);
	printf("bye\n");
	return 0;
}
