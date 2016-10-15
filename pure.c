#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main(int argc, char *argv[]) {
	initialize();

	Object *readExp, *evalExp;
	Object *env;
	FILE *fp = stdin;

	// ����������΃\�[�X�t�@�C����ǂݍ���
	if (argc == 2) {
		if (strcmp(argv[1], "-e") == 0) {
			// -e�I�v�V�����͕W�����͂œǂݍ��݂ƕ]�������ɂ���
			// (������̊��K�ƈႤ���ǃe�X�g���y�Ȃ̂�)
		} else {
			fp = fopen(argv[1], "r");
		}
	}

	if (argc == 2) {
		// �t�@�C���w�肠��.�ǂݍ��݂ƕ]������
		while (1) {
			readExp = read(TopEnv, fp);
			if (readExp == NULL) {
				break;
			}
			evalExp = eval(TopEnv, readExp);
		}
	} else {
		// �t�@�C���w�肳��Ă��Ȃ���ΑΘb���ۂ�
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
