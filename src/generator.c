#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>
#define FREQUENCY_SIZE 5
char* generate_address(char option);

char values[16]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

int main(){
  int i;
  int size;                    /*the size of the file*/
  int offset, accesses;        /*the size of the repeted addresses array and the number of the accesses in that array.
				 small offset and big access ---> big frequency*/

  int buffer_token;            /*a token shows how many accesses do i have from the rep_address buffer*/
  int new_token;               /*a token shows how many writes do i have from the generator. (buffer_token+new_token==size)*/

  char option;                 /*the option. L for load only, S for store only, B for both*/
  char **rep_addresses;        /*a buffer initialized with random addresses which will be written mulltiple times*/
  char do_it;                  /*just a flag for the main loop*/
  char x;                      /*stores a random binary value (0 or 1) to choose the input stream. Buffer or generator*/
  int index;                   /*the index for the rep_address buffer*/

  FILE *fp;

  srand(time(0));
  printf("Give the size: ");
  scanf("%d", &size);
  scanf("%c", &option);        /*to capture the enter button :( */
  printf("Give your option: ");
  scanf("%c", &option);
  printf("FOR FREQUENCY. give the offset: ");
  scanf("%d", &offset);
  printf("give the accesses: ");
  scanf("%d", &accesses);

  fp=fopen("mem_file", "w");
  assert(fp!=NULL);
  rep_addresses=(char**)malloc(sizeof(char*)*offset);

  buffer_token=accesses;    /*the token to write from the buffer*/
  new_token=size-accesses;  /*the token to write new. Aka not from buffer.*/
  
  /*fill the buffer with the repeted addresses*/
  for(i=0; i<offset; i++)
    rep_addresses[i]=generate_address(option);

  /*write size-accesses random addresses in the file and write accesses addresses from the buffer in the file*/
  /*first choose if you will write from the buffer of a new random access*/
  do_it=buffer_token+new_token;
  while(do_it){                     /*we want a total of size addresses in the file. Some of them are from the buffer and some of them new*/
    x=rand()%2;
    if(x){                                                        /*write a random address from the buffer*/
      if(buffer_token>0){                                         /*you have accesses in the buffer so do it*/
	index=rand()%offset;
	if(option=='B')
	  if(rand()%2)
	    rep_addresses[index][0]='L';
	  else
	    rep_addresses[index][0]='S';
	fprintf(fp, "%s", rep_addresses[index]);
	buffer_token--;                                           /*but now you have an access less*/
      }else
	continue;
    }
    
    if(!x){                                                       /*write a new random addresss in the file*/
      if(new_token>0){
	fprintf(fp, "%s", generate_address(option));
	new_token--;
      }else
	continue;
    }
    do_it=new_token+buffer_token;                                  /*if both of them are zero then <size> writes in the file happened
								    <Access> of them from the buffer and the rest from the generator*/
    if(do_it!=0)
      fprintf(fp, "\n");
  }
  fclose(fp);
  free(rep_addresses);
  return 0;
}


char* generate_address(char option){
  int j;
  char *address=(char*)malloc(13);
  address[1]=32;      /*there is allways a space between instruction and address*/
  /*first write the instruction*/
    switch(option){
    case 'L':
      address[0]='L';
      break;
    case 'S':
      address[0]='S';
      break;
    case 'B':
      if(rand()%2)
	address[0]='L';
      else
	address[0]='S';
      break;
    default:
      assert(0);
      break;
    }
    address[2]='0';
    address[3]='x';
    
    /*now write the address*/
    for(j=0; j<8; j++)                         /*address consists from 8 bytes*/
      address[j+4]=values[rand()%16];          /*give a random number in hex*/
    address[j+4]=0;                            /*null the string*/
    return address;
}
