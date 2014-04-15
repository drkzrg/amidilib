
#include <stdio.h>
#include "midi2nkt.h"


void printInfoScreen();

int main(int argc, char *argv[]){

    if(argc>=1&&argv[1]!='\0'){
         fprintf(stderr,"Trying to load %s\n",argv[1]);
     }else{
         fprintf(stderr,"No specified mid filename! exiting\n");
         return 0;
     }

  //load mid

  //check mid 0, no quit

  printInfoScreen();

  // convert


  //done..
}


void printInfoScreen(){


    printf("\n== MID to NKT converter v.1.00 =========\n");
    printf("date: %s %s\n",__DATE__,__TIME__);

    printf("==========================================\n");
    printf("Ready...\n");
}
