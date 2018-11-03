#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(int argc, char* argv[]){
	if(argc>2){
    		FILE *fp1;
    		FILE *fp2;
    		char str[1];
    		char* filename1;
		char* filename2;
		int counter=0;
		char ch;
		int am;
		int an;
		int bm;
		int bn;
		int i=0;
		char str1[10], str2[10];
    		filename1=argv[1];
    		filename2=argv[2];

		printf("%s\n",filename1);
		printf("%s\n",filename2);

    		fp1 = fopen(filename1, "r");
    		fp2 = fopen(filename2, "r");

    		if (fp1 == NULL){
        		printf("Could not open file %s",filename1);
    		}
    		if (fp2 == NULL){
        		printf("Could not open file %s",filename2);
    		}
		fscanf(fp1,"rows(%d) cols(%d)",&am,&an);
		printf("rows = %d, cols = %d\n",am,an);
    		fclose(fp1);
    		fclose(fp2);
	}
	return 0;
}
