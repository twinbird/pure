#include <stdio.h>
#include "pureLisp.h"

int main(int argc, char *argv[]) {
	initialize();

	Object *readExp, *evalExp;
	Object *env;
	FILE *fp = stdin;

	// 引数があればソースファイルを読み込む
	if (argc == 2) {
		fp = fopen(argv[1], "r");
	}

	if (argc == 2) {
		// ファイル指定あり.読み込みと評価だけ
		while (1) {
			readExp = read(TopEnv, fp);
			if (readExp == NULL) {
				break;
			}
			evalExp = eval(TopEnv, readExp);
		}
	} else {
		// ファイル指定されていなければ対話っぽく
		while (1) {
			printf(">");
			readExp = read(TopEnv, fp);
			if (readExp == NULL) {
				break;
			}
			evalExp = eval(TopEnv, readExp);
			print(evalExp);
			printf("\n");
		}
	}

	fclose(fp);

	return 0;
}
