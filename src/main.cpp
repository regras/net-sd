/*
 * File:   main.c
 * Author: Frank Breitinger
 * Created on 28. April 2013, 19:15
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <openssl/md5.h>
#include <limits.h>
#include <vector>
#include <iostream>
#include <dirent.h>
#include<sys/types.h>
#include<errno.h>
#include "boost/filesystem.hpp"
#include "../header/main.h"
#include "../header/helper.h"
#include "../header/sdhash.h"
#include "../header/hashing.h"
#include "../header/bloomfilter.h"
using std::cout;
using namespace boost::filesystem;
// Global variable for the different modes


MODES *mode;
extern char *optarg;
extern int optind;

static int counter=0;
char LINE[500];
FILE *list;



static void show_help(void) {
	printf(
			"\nnet-sd  by Frank Breitinger and modified by João Pedro Bizzi Velho\n"
					"Copyright (C) 2013 \n"
					"\n"
					"Usage: net_sd [-gdcih]  DIR/FILE \n"
					"OPTIONS: -g: Generate the database and print it to std (save to a file like: './mrsh_sd DIR > myDB, if it's a dir: ./dirname/* \n"
					"         -d: Generate the database using a list of files './mrsh-net -d list.txt' \n"
					"					-c: Reads DB-FILE and compares DIR/FILE against it ./mrsh_sd -c <digest> ./dir/*"
					"         -i: Reads DB-FILE and compares it against a list of FILES/DIRS, './mrsh-net -i digestname list.txt . \n"
					"         -h: Print this help message \n\n");
}


static void initalizeDefaultModes() {
	mode = (MODES *) malloc(sizeof(MODES));
	mode->helpmessage = false;
	mode->generateBF = false;
	mode->readDB = false;
	mode->compare = false;
	mode->list = false;
}


int main(int argc, char **argv) {

	int i;
	initalizeDefaultModes();

	char *listName = NULL;

	while ((i = getopt(argc, argv, "i:c:ghd:a")) != -1) {
		switch (i) {
		case 'd':
			mode->list = true;
			break;
		case 'c':
			mode->compare = true;
			listName = optarg;
		case 'i':
			mode->readDB = true;
			listName = optarg;
			break;
			case 'g':
			mode->generateBF = true;
			break;
		case 'h':
			mode->helpmessage = true;
			break;
		default:
			mode->helpmessage = true;
			fprintf(stderr, "[*] Unknown option(s) \n");
			break;
		}
	}

	if (mode->helpmessage) {
		show_help();
		exit(0);
	}

	initialize_settings();		//Bloom filter settings

	BLOOMFILTER *bf = init_empty_BF();

	/*read db from file and compare stuff against it (not a list)*/
	if(mode->compare){

	// Reads the BF to the memory
	readFileToBF(listName, bf);
	int size;

	// lê cada arquivo
	for (int j = optind; j < argc; j++) {

		FILE *file = getFileHandle(argv[j]);
		size = find_file_size(file);

		// busca este arquivo no filtro de bloom carregado na memória
		evaluation(bf,size,argv[j]);

		fclose(file);

	}
	destroy_bf(bf);

	exit(1);
}

	// read db from file and compare stuff against it (w. a list)
	if(mode->readDB) {

		// Reads the BF to the memory
		readFileToBF(listName, bf);
		unsigned char *buffer;
		int size;

		list = fopen(argv[3],"r");
		int total_de_linhas = 0;

		// Find the # of lines
		while(fgets(LINE,500,list))total_de_linhas++;
		fseek(list, 0 , SEEK_SET);

		// Find the filename in each line and search the file in bf
		for (int j = 1	; j <= total_de_linhas; j++) {
			fgets (LINE, 500 ,list);
			char *filename;
			filename = strtok(LINE,"\n");

			FILE *file = getFileHandle(filename);
			size = find_file_size(file);
		//	printf("filename #%i = %s\n",j,filename);
			evaluation(bf,size,filename);

			fclose(file);

		}
		destroy_bf(bf);

		exit(1);
	}

	//read all arguments, create the bloomfilter, and print it to stdout
	if (mode->generateBF || optind == 1) {
		int size;

		//fill bloom filter with all files
		for (int j = optind; j < argc; j++) {
			//printf("filename # %i = %s\n",j-optind + 1,argv[j]);
			FILE *file = getFileHandle(argv[j]);
			size = find_file_size(file);

			hashFileAndDo(bf, 1, 0, size,argv[j]);

			fclose(file);

		}
		// Clean up
		print_bf(bf);
		destroy_bf(bf);
		exit(1);
}

	// Creates new bf from a list of files
	if (mode-> list){

		// Find the # of lines = # of files to search
		list = fopen(argv[2],"r");
		int total_de_linhas = 0;

		while(fgets(LINE,500,list)){
			total_de_linhas++;
			char *filename;
			filename = strtok(LINE,"\n");
			FILE *temporary;
			temporary = getFileHandle(filename);
			fclose(temporary);
		}

		fseek(list, 0 , SEEK_SET);

		int size;

		// Inserting every file listed
		for (int j = 1	; j <= total_de_linhas; j++) {
			fgets (LINE, 500 ,list);
			char *filename;
			filename = strtok(LINE,"\n");

			FILE *file = getFileHandle(filename);
			size = find_file_size(file);

			//printf("filename # %i = %s\n",j,LINE);

			hashFileAndDo(bf, 1, 0, size,LINE);

			fclose(file);
		}


		print_bf(bf);
		destroy_bf(bf);
		exit(1);

	}


}

/* Função que faz a comparação entre os objetos inseridos */
void evaluation(BLOOMFILTER *bf,int size,char *filename) {
	unsigned int found = 0,total = 0,longest_run = 0;

	int *results;

	results = hashFileAndDo(bf, 2, 0, size,filename);
	found = *(results+1);
	total = *(results)+ found;
	longest_run = *(results+2);

	if (longest_run >= MIN_RUN)	printf("%s:%d of %d(Longest run %d) ===== %d\n", filename, found, total,longest_run,total - found);
	else printf("%s not found, min long run not long enough \n",filename);
}
