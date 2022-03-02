#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <math.h>

/* macros */
#define VERSION	"coinC version 0.0.1"
#define WBY	"written by dimartz"
#define DEFIND	"10"
#define MAXIND	"2000"
#define URL	"api.coincap.io/v2/assets?limit="
#define PATH	".cache/statusbar"
#define FILETMP	"coins.tmp"
/* colors */
#define RED	"\x1b[31m"
#define GREEN	"\x1b[32m"
#define YELLOW	"\x1b[33m"
#define BLUE	"\x1b[34m"
#define MAGENTA	"\x1b[35m"
#define CYAN	"\x1b[36m"
#define RESET	"\x1b[0m"

/* variables or pointers */
char *argv1 = NULL;
char *argv2 = NULL;
char *columns[8] = {	"Rank", "Coin", "Price (USD)", "Change 24h",
			"Market Cap", "Supply", "Volume 24h"		};

/* data structures */
struct CoinData {
	unsigned rank;
	char symbol[8];
	long double price;
	long double change24h;
	long double marketCap;
	long double supply;
	long double volume24h;
	struct CoinData *next;
};

struct ColLen {
	unsigned len;
	struct ColLen *next;
};

struct MemoryStruct {
	char *mem;
	size_t size;
};

/* function declarations */
unsigned optionCheck();
void topCoins();
unsigned loadIndex();
void *curlCoincap(unsigned);
static size_t writeCurlback(void*, size_t, size_t, void*);
struct CoinData *fillTopCoinData(char*, unsigned);
void *searchCoinData(char*, char*);
struct CoinData *createCoin();
struct CoinData *appendCoin(struct CoinData*, struct CoinData*);
void freeCoinData(struct CoinData*);
void *ptrRewind(char*);
void printTable(struct CoinData*);
struct ColLen *calcColLen(struct CoinData*);
struct ColLen *createColLen();
struct ColLen *appendColLen(struct ColLen*, struct ColLen*);
void freeColLen(struct ColLen*);
unsigned intCount(long double);
double formatNum(long double);
void formattedSym(long double, char*);
void printHeaderTable(struct ColLen*);
void printMiddleTable(struct ColLen*);
void printBottomTable(struct ColLen*);
void findCoin();
struct CoinData *fillFindCoinData(char*);
void loadCoin(char*);
void toUpperString(char*);
void makePath();
void printPrice();
void printErrN(unsigned);
void printErrArg(unsigned, char*);
void printMoreInfo();
void printVersion();
void printHelp();

int
main(int argc, char *argv[]) {
	/* functions access these variables
	 * from these external pointers */
	argv1 = *(argv+1);
	argv2 = *(argv+2);

	/* standard output */
	if (argc == 1) {
		argv2 = DEFIND;
		topCoins();
	} else {
		if (*argv1 != '-') {
			printErrArg(22, argv1);
			printMoreInfo();
		} else {
			/* options */
			switch (optionCheck()) {
				case 'f':
					if (argc > 3) {
						printErrN(7);
						printMoreInfo();
						break;
					} else if (!argv2) {
						printErrN(22);
						printMoreInfo();
						break;
					}
					findCoin();
					break;
				case 't':
					if (argc > 3) {
						printErrN(33);
						printMoreInfo();
						break;
					} else if (!argv2)
						argv2 = DEFIND;
					topCoins();
					break;
				case 'p': /* hidden option */
					if (argc > 3) {
						printErrN(7);
						printMoreInfo();
						break;
					} else if (!argv2) {
						printErrN(22);
						printMoreInfo();
						break;
					}
					makePath();
					printPrice();
					break;
				case 'v':
					if (argc > 2) {
						printErrN(7);
						printMoreInfo();
						break;
					}
					printVersion();
					break;
				case 'h':
					if (argc > 2) {
						printErrN(7);
						printMoreInfo();
						break;
					}
					printHelp();
					break;
				default:
					printErrArg(22, argv1);
					printMoreInfo();
			}
		}
	}
	exit(EXIT_SUCCESS);
}

unsigned
optionCheck() {
	if	(!strcmp(argv1, "-f")	|| !strcmp(++argv1, "-find"))
		return 'f';
	else if (!strcmp(--argv1, "-p") || !strcmp(++argv1, "-price"))
		return 'p';
	else if (!strcmp(--argv1, "-t") || !strcmp(++argv1, "-top"))
		return 't';
	else if (!strcmp(--argv1, "-v") || !strcmp(++argv1, "-version"))
		return 'v';
	else if (!strcmp(--argv1, "-h") || !strcmp(++argv1, "-help"))
		return 'h';
	else
		--argv1;
	return '\0';
}

void
topCoins() {
	unsigned i, index = loadIndex();
	char *memptr;

	printf("Connecting to coincap.io server...");
	for (i = 1; !(memptr = curlCoincap(index)) && i <= 10; i++)
		;
	if (i == 1)
		printf("(1 call) ");
	else
		printf("(%d calls) ", i);
	if (!memptr) {
		printf("\n");
		printErrN(111);
		exit(EXIT_FAILURE);
	} else
		printf("%s!\n", strerror(0));
	struct CoinData *start = fillTopCoinData(memptr, index);
	printTable(start);
	freeCoinData(start);
	memptr = ptrRewind(memptr);
	free(memptr);
}

unsigned
loadIndex() {
	while (*argv2) {
		if(!isdigit(*argv2)) {
			printErrArg(22, argv2);
			printMoreInfo();
			exit(EXIT_FAILURE);
		} else
			++argv2;
	}
	argv2 = ptrRewind(argv2);
	unsigned index = atoi(argv2);
	if (index == 0)
		index = 10;
	else if (index > 2000)
		index = 2000;
	return index;
}

void *
curlCoincap(unsigned index) {
	char limit[] = MAXIND;
	sprintf(limit, "%d", index);
	char customURL[strlen(URL)+strlen(limit)];
	customURL[0] = '\0';
	strcat(customURL, URL);
	strcat(customURL, limit);

	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.mem = malloc(1);
	chunk.size = 0;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_URL, customURL);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headers = NULL;
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "\ncurl_easy_perform() error: %s\n",
					curl_easy_strerror(res));
		else
			if ((unsigned long)chunk.size <= 61)
				return NULL;
	}
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return chunk.mem;
}

static size_t
writeCurlback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	char *ptr = realloc(mem->mem, mem->size + realsize + 1);

	if (!ptr) {
		printErrN(12);
		return 0;
	}
	mem->mem = ptr;
	memcpy(&(mem->mem[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->mem[mem->size] = 0;
	return realsize;
}

struct CoinData *
fillTopCoinData(char *memptr, unsigned index) {
	struct CoinData *start = NULL, *new, *end;
	char aux[BUFSIZ];
	unsigned rank = 0;

	while (rank < index) {
		strcpy(aux, "rank");
		memptr = searchCoinData(aux, memptr);
		sscanf(aux, "%u", &rank);
		if (rank <= index) {
			new = createCoin();
			new->rank = rank;
		}
		strcpy(aux, "symbol");
		memptr = searchCoinData(aux, memptr);
		strcpy(new->symbol, aux);
		strcpy(aux, "supply");
		memptr = searchCoinData(aux, memptr);
		sscanf(aux, "%Lf", &new->supply);
		strcpy(aux, "marketCapUsd");
		memptr = searchCoinData(aux, memptr);
		sscanf(aux, "%Lf", &new->marketCap);
		strcpy(aux, "volumeUsd24Hr");
		memptr = searchCoinData(aux, memptr);
		sscanf(aux, "%Lf", &new->volume24h);
		strcpy(aux, "priceUsd");
		memptr = searchCoinData(aux, memptr);
		sscanf(aux, "%Lf", &new->price);
		strcpy(aux, "changePercent24Hr");
		memptr = searchCoinData(aux, memptr);
		sscanf(aux, "%Lf", &new->change24h);
		if (!start) {
			start = new;
			end = start;
		} else
			end = appendCoin(end, new);
	}
	return start;
}

void *
searchCoinData(char *aux, char *memptr) {
	char tmp[BUFSIZ];
	unsigned i;

	while (strcmp(aux, tmp)) {
		while (!isalpha(*memptr) && !isdigit(*memptr))
			++memptr;
		for (i = 0; *memptr != '\"'; i++, memptr++)
			tmp[i] = *memptr;
		tmp[i] = '\0';
	}
	while (!isalpha(*memptr) && !isdigit(*memptr) && *memptr != '-')
		++memptr;
	for (i = 0; *memptr != '\"' && *memptr != ','; i++, memptr++)
		aux[i] = *memptr;
	aux[i] = '\0';
	if(!strcmp(aux, "null"))
		strcpy(aux, "0");
	return memptr;
}

struct CoinData *
createCoin() {
	struct CoinData *ptr;
	ptr = (struct CoinData *) malloc(sizeof(struct CoinData));
	ptr->next = NULL;
	return ptr;
}

struct CoinData *
appendCoin(struct CoinData *end, struct CoinData *new) {
	end->next = new;
	return end->next;
}

void
freeCoinData(struct CoinData *start) {
	struct CoinData *ptr = start;
	struct CoinData *tmp = NULL;
	while (ptr) {
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
	}
}

void *
ptrRewind(char *ptr) {
	while(*--ptr)
		;
	return ++ptr;
}

void
printTable(struct CoinData *start) {
	struct CoinData *ptr = start;
	struct ColLen *colstart;

	colstart = calcColLen(start);
	printHeaderTable(colstart);
	while (ptr) {
		char middle[] = "│";
		unsigned i, aux, space;
		struct ColLen *col = colstart;
		printf(BLUE "%s" RESET, middle);
		space = 1;
		aux = col->len - intCount(ptr->rank) - space;
		for (i = 0; i < aux; i++)
			printf(" ");
		printf("%u", ptr->rank);
		printf(BLUE " %s" RESET, middle);
		col = col->next;
		aux = col->len - strlen(ptr->symbol);
		if (aux % 2 != 0 && strlen(ptr->symbol) < 4)
			printf(" ");
		for (i = 0; i < aux/2; i++)
			printf(" ");
		printf("%s", ptr->symbol);
		for (i = 0; i < aux/2; i++)
			printf(" ");
		if (aux % 2 != 0 && strlen(ptr->symbol) > 4)
			printf(" ");
		printf(BLUE "%s" RESET, middle);
		col = col->next;
		space = 6;
		aux = col->len - (intCount(ptr->price) + space);
		for (i = 0; i < aux; i++)
			printf(" ");
		printf("%.4Lf", ptr->price);
		printf(BLUE " %s" RESET, middle);
		col = col->next;
		aux = col->len - (intCount(ptr->change24h) + space);
		for (i = 0; i < aux; i++)
			printf(" ");
		if (ptr->change24h > 0)
			printf(GREEN "+");
		else if (ptr->change24h < 0)
			printf(RED);
		else
			printf(" ");
		printf("%.2Lf%%", ptr->change24h);
		printf(BLUE " %s" RESET, middle);
		col = col->next;
		space = 4;
		aux = col->len - (intCount(formatNum(ptr->marketCap)) + space);
		for (i = 0; i < aux; i++)
			printf(" ");
		char symbol[32];
		formattedSym(ptr->marketCap, symbol);
		printf("%s", symbol);
		printf(BLUE " %s" RESET, middle);
		col = col->next;
		aux = col->len - (intCount(formatNum(ptr->supply)) + space);
		for (i = 0; i < aux; i++)
			printf(" ");
		formattedSym(ptr->supply, symbol);
		printf("%s", symbol);
		printf(BLUE " %s" RESET, middle);
		col = col->next;
		aux = col->len - (intCount(formatNum(ptr->volume24h)) + space);
		for (i = 0; i < aux; i++)
			printf(" ");
		formattedSym(ptr->volume24h, symbol);
		printf("%s", symbol);
		printf(BLUE " %s" RESET, middle);
		printf("\n");
		if (ptr->next)
			printMiddleTable(colstart);
		else
			printBottomTable(colstart);
		ptr = ptr->next;
	}
	freeColLen(colstart);
}

struct ColLen *
calcColLen(struct CoinData *start) {
	struct CoinData *ptr = start;
	struct ColLen *colstart, *colnew, *colend;
	unsigned i, aux;

	colnew = createColLen();
	colnew->len = strlen(MAXIND);
	colstart = colnew;
	colend = colstart;
	for (i = 1; columns[i]; i++) {
		colnew = createColLen();
		colnew->len = strlen(columns[i]);
		colend = appendColLen(colend, colnew);
	}
	while (ptr) {
		struct ColLen *col = colstart;
		col = col->next;
		if (strlen(ptr->symbol) > col->len)
			col->len = strlen(ptr->symbol);
		col = col->next;
		unsigned space = 7;
		aux = intCount(ptr->price) + space;
		if (aux - 2 > col->len)
			col->len = aux;
		col = col->next;
		ptr->change24h = roundf(ptr->change24h * 100) / 100;
		aux = intCount(ptr->change24h) + space;
		if (aux - 2 > col->len)
			col->len = aux;
		col = col->next;
		space = 5;
		aux = intCount(formatNum(ptr->marketCap)) + space;
		if (aux - 2 > col->len)
			col->len = aux;
		col = col->next;
		aux = intCount(formatNum(ptr->supply)) + space;
		if (aux - 2 > col->len)
			col->len = aux;
		col = col->next;
		aux = intCount(formatNum(ptr->volume24h)) + space;
		if (aux - 2 > col->len)
			col->len = aux;
		ptr = ptr->next;
	}
	return colstart;
}

struct ColLen *
createColLen() {
	struct ColLen *ptr;
	ptr = (struct ColLen *) malloc(sizeof(struct ColLen));
	ptr->next = NULL;
	return ptr;
}

struct ColLen *
appendColLen(struct ColLen *end, struct ColLen *new) {
	end->next = new;
	return end->next;
}

void
freeColLen(struct ColLen *start) {
	struct ColLen *ptr = start;
	struct ColLen *tmp = NULL;
	while (ptr) {
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
	}
}

unsigned
intCount(long double num) {
	unsigned i;

	if (num < 0)
		num *= -1;
	for (i = 1; num >= 10; i++)
		num /= 10;
	return i;
}

double
formatNum(long double num) {
	if (num >= 1e12)
		num /= 1e12;
	else if (num >= 1e9)
		num /= 1e9;
	else if (num >= 1e6)
		num /= 1e6;
	else if (num >= 1e3)
		num /= 1e3;
	return (roundf(num * 10) / 10);
}

void
formattedSym(long double num, char *sym) {
	if (num >= 1e12) {
		num = roundf(formatNum(num) * 10) / 10;
		snprintf(sym, 32, "%.1Lf", num);
		strcat(sym, "T");
	} else if (num >= 1e9) {
		num = roundf(formatNum(num) * 10) / 10;
		snprintf(sym, 32, "%.1Lf", num);
		strcat(sym, "B");
	} else if (num >= 1e6) {
		num = roundf(formatNum(num) * 10) / 10;
		snprintf(sym, 32, "%.1Lf", num);
		strcat(sym, "M");
	} else if (num >= 1e3) {
		num = roundf(formatNum(num) * 10) / 10;
		snprintf(sym, 32, "%.1Lf", num);
		strcat(sym, "K");
	} else {
		snprintf(sym, 32, "%.1Lf", num);
		strcat(sym, " ");
	}
}

void
printHeaderTable(struct ColLen *start) {
	char top_left[]		= "┌";
	char top_mid[]		= "┬";
	char top_right[]	= "┐";
	char middle[]		= "│";
	char left_mid[]		= "├";
	char mid[]		= "─";
	char mid_mid[]		= "┼";
	char right_mid[]	= "┤";
	unsigned i, j, aux;
	struct ColLen *ptr;

	printf(BLUE "\n%s", top_left);
	for (i = 0, ptr = start; columns[i]; i++, ptr = ptr->next) {
		aux = strlen(columns[i]);
		if (ptr->len == aux)
			ptr->len += 2;
		else if (ptr->len > aux) {
			if ((ptr->len % 2 == 0 && aux % 2 != 0) ||
				(ptr->len % 2 != 0 && aux % 2 == 0))
				++ptr->len;
			else
				ptr->len += 2;
		}
		for (j = 0; j < ptr->len; j++)
			printf("%s", mid);
		if (columns[i+1])
			printf("%s", top_mid);
	}
	printf("%s", top_right);
	printf("\n");
	printf("%s", middle);
	for (i = 0, ptr = start; columns[i]; i++, ptr = ptr->next) {
		aux = ptr->len - strlen(columns[i]);
		for (j = 0; j < aux/2; j++)
			printf(" ");
		printf(YELLOW "%s", columns[i]);
		for (j = 0; j < aux/2; j++)
			printf(" ");
		if (columns[i+1])
			printf(BLUE "%s", middle);
	}
	printf(BLUE "%s", middle);
	printf("\n");
	printf("%s", left_mid);
	for (i = 0, ptr = start; columns[i]; i++, ptr = ptr->next) {
		for (j = 0; j < ptr->len; j++)
			printf("%s", mid);
		if (columns[i+1])
			printf("%s", mid_mid);
	}
	printf("%s", right_mid);
	printf("\n" RESET);
}

void
printMiddleTable(struct ColLen *start) {
	char left_mid[]		= "├";
	char mid[]		= "─";
	char mid_mid[]		= "┼";
	char right_mid[]	= "┤";
	unsigned i, j;
	struct ColLen *ptr;

	printf(BLUE "%s", left_mid);
	for (i = 0, ptr = start; columns[i]; i++, ptr = ptr->next) {
		for (j = 0; j < ptr->len; j++)
			printf("%s", mid);
		if (columns[i+1])
			printf("%s", mid_mid);
	}
	printf("%s", right_mid);
	printf("\n" RESET);
}

void
printBottomTable(struct ColLen *start) {
	char bottom_left[]	= "└";
	char bottom[]		= "─";
	char bottom_mid[]	= "┴";
	char bottom_right[]	= "┘";
	unsigned i, j;
	struct ColLen *ptr;

	printf(BLUE "%s", bottom_left);
	for (i = 0, ptr = start; columns[i]; i++, ptr = ptr->next) {
		for (j = 0; j < ptr->len; j++)
			printf("%s", bottom);
		if (columns[i+1])
			printf("%s", bottom_mid);
	}
	printf("%s", bottom_right);
	printf("\n" RESET);
}

void
findCoin() {
	unsigned i, index;
	sscanf(MAXIND, "%u", &index);
	char *memptr;

	printf("Connecting to coincap.io server...");
	for (i = 1; !(memptr = curlCoincap(index)) && i <= 10; i++)
		;
	if (i == 1)
		printf("(1 call) ");
	else
		printf("(%d calls) ", i);
	if (!memptr) {
		printf("\n");
		printErrN(111);
		exit(EXIT_FAILURE);
	} else
		printf("%s!\n", strerror(0));
	struct CoinData *start = fillFindCoinData(memptr);
	if (start)
		printTable(start);
	else
		printf(RED "\nNo coins found matching your keywords.\n");
	freeCoinData(start);
	memptr = ptrRewind(memptr);
	free(memptr);
}

struct CoinData *
fillFindCoinData(char *memptr) {
	struct CoinData *start = NULL, *new, *end;
	char aux[BUFSIZ];
	char coin[32];
	unsigned rank = 0, index;
	sscanf(MAXIND, "%u", &index);

	while (*argv2) {
		loadCoin(coin);
		while(rank < index) {
			strcpy(aux, "rank");
			memptr = searchCoinData(aux, memptr);
			sscanf(aux, "%u", &rank);
			strcpy(aux, "symbol");
			memptr = searchCoinData(aux, memptr);
			if (!strcmp(aux, coin)) {
				new = createCoin();
				strcpy(new->symbol, aux);
				new->rank = rank;
				strcpy(aux, "supply");
				memptr = searchCoinData(aux, memptr);
				sscanf(aux, "%Lf", &new->supply);
				strcpy(aux, "marketCapUsd");
				memptr = searchCoinData(aux, memptr);
				sscanf(aux, "%Lf", &new->marketCap);
				strcpy(aux, "volumeUsd24Hr");
				memptr = searchCoinData(aux, memptr);
				sscanf(aux, "%Lf", &new->volume24h);
				strcpy(aux, "priceUsd");
				memptr = searchCoinData(aux, memptr);
				sscanf(aux, "%Lf", &new->price);
				strcpy(aux, "changePercent24Hr");
				memptr = searchCoinData(aux, memptr);
				sscanf(aux, "%Lf", &new->change24h);
				if (!start) {
					start = new;
					end = start;
				} else
					end = appendCoin(end, new);
			}
		}
		rank = 0;
		memptr = ptrRewind(memptr);
	}
	return start;
}

void
loadCoin(char coin[]) {
	if (*argv2 == ',')
		++argv2;
	unsigned i;
	for (i = 0; *argv2 && *argv2 != ','; i++)
		coin[i] = *argv2++;
	coin[i] = '\0';
	toUpperString(coin);
}

void
toUpperString(char string[]) {
	unsigned i;
	for (i = 0; string[i] != '\0'; i++)
		string[i] = toupper(string[i]);
}

void
makePath() {
	char cmd[] = "mkdir -p ";
	char mkp[strlen(cmd)+strlen(PATH)];
	mkp[0] = '\0';
	strcat(mkp, cmd);
	strcat(mkp, PATH);

	chdir(getenv("HOME"));
	system(mkp);
	chdir(PATH);
}

void
printPrice() {
	if (!strcmp(argv2, "btc")) {
		unsigned rank = 0, index;
		sscanf(MAXIND, "%u", &index);
		char *memptr;
		while(!(memptr = curlCoincap(index)))
			;
		FILE *write;
		write = fopen(FILETMP, "w");
		char aux[BUFSIZ];

		while (rank < index) {
			strcpy(aux, "rank");
			memptr = searchCoinData(aux, memptr);
			sscanf(aux, "%u", &rank);
			strcpy(aux, "symbol");
			memptr = searchCoinData(aux, memptr);
			fprintf(write, "%s\n", aux);
			strcpy(aux, "priceUsd");
			memptr = searchCoinData(aux, memptr);
			fprintf(write, "%s\n", aux);
		}
		fclose(write);
	}
	char aux[BUFSIZ];
	char coin[8];
	FILE *read;
	read = fopen(FILETMP, "r");
	loadCoin(coin);
	while (strcmp(coin, aux) && (fscanf(read, "%s", aux) != EOF))
		;
	if (strcmp(coin, aux)) {
		printf(RED "\nNo coins found matching your keywords.\n");
		exit(EXIT_FAILURE);
	}
	fscanf(read, "%s", aux);
	printf("%s", aux);
	fclose(read);
}

void
printErrN(unsigned nerror) {
	printf("\nError(%d): %s.\n", nerror, strerror(nerror));
}

void
printErrArg(unsigned nerror, char *argvn) {
	argvn = ptrRewind(argvn);
	printf("\nError(%d) \"%s\": %s.\n", nerror, argvn, strerror(nerror));
}

void
printMoreInfo() {
	printf("\nTry 'coinc --help' for more information.\n");
}

void
printVersion() {
	printf("\n%s\n%s\n", VERSION, WBY);
}

void
printHelp() {
	printf("\nUsage: coinc [OPTION]\n \
		\nOptions:\n \
		\r  -f, --find [SYMBOL]\tfind specific coin data with \
coin symbol (can a comma separated list) (default: [])\n \
		\r  -t, --top [INDEX]\tshow the top coins ranked from \
1 - [INDEX] according to the market cap (default: 10)\n \
		\r  -v, --version\t\toutput the version number\n \
		\r  -h, --help\t\toutput usage information\n");
}
