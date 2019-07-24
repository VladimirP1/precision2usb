#include <fileio.h>
#include <parser.h>
#include <precision.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void parse_report_desc(uint8_t* buf, size_t len, prec_config* movers) {
	token tok;
	tokenizer_state tS;
	parser_state pS;
	token_init(&tS, buf, len);
	parser_init(&pS);
	while(!token_next(&tS, &tok)) {
		parser_token(&pS, &tok);
	}

	ast_print(pS.rootNode);
	prec_init_from_ast(pS.rootNode, movers);

	parser_deinit(&pS);
}

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Usage: main <hid descriptor file>\n");
        return 1;
    }

    size_t len = 0;
    uint8_t* data = read_all(argv[1], &len);

    prec_config cfg;
    memset(&cfg, 0, sizeof(cfg));

    parse_report_desc(data, len, &cfg);

    free(data);
}
