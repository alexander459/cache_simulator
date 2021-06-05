#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<time.h>

struct cache_line{  /*it represents a line of a set in the cache memory*/
  short valid_bit;  /*the valid bit*/
  long int tag;          /*the address tag*/
  char *block;      /*the block. char type. allocates space <number of bytes in block>*/
  struct cache_line *next; /*the next line of the set*/
};

struct cache_memory{         /*it represents the whole cache memory.*/
  int number_of_sets;        /*the number of the total sets*/
  int number_of_blocks;      /*the number of the blocks per set*/
  struct cache_line** sets;  /*a double pointer points at the beggining of the hash table*/
}memory;

int *fifo_array;             /*array contains for every set the next block to be replaced with fifo algorithm*/
int **lru_array;             /*array contains for every set the frequency of the accesses at every block*/
int *accesses;

void print_cache();
struct cache_line* new_block(int bytes);
void allocate_the_cache(int sets, int blocks, int bytes);
int is_power_of_two(int number);
void print_usage(void);

int load_address(long int address);
void lru_rep(long int address);
void fifo_rep(long int address);
void random_rep(long int address);
void accessed(long int address);
void free_all(char algorithm);

int main(int argc, char** argv){
  /*Args: number of sets, number of blocks per set, number of bytes per block, replacement algorithm*/
  int number_of_sets;
  int number_of_blocks;
  int number_of_bytes;
  short algorithm;

  void (*replace[3])(long int address)={lru_rep, fifo_rep, random_rep};  /*contains the replace algorithm functions*/

  char instruction;
  char str[15];
  long int address;
  int l_hits=0, l_misses=0;
  int i, j;
  FILE *fp;
  char f_name[100];
  printf("Name the file: ");
  scanf("%100s", f_name);
  fp=fopen(f_name, "r");
  assert(fp!=NULL);

  srand(time(0));
  /*ARGUMENTS MUST BE 5. a.out AND THE 4 ARGUMENTS FOR THE PROGRAM FUNCTIONALITY*/
  if(argc!=5){
    printf("4 arguments required. Usage:\n");  /*there is an error in the input*/
    print_usage();
    printf("aborting...\n");
    return 0;
  }

  /*convert the string-numbers to integers*/
  number_of_sets=atoi(argv[1]);
  number_of_blocks=atoi(argv[2]);
  number_of_bytes=atoi(argv[3]);

  /*CHECK IF THE NUMBER-ARGUMENTS ARE VALID*/
  if(!is_power_of_two(number_of_sets) || !is_power_of_two(number_of_blocks) || !is_power_of_two(number_of_bytes) || number_of_bytes<4){
    printf("invalid input. Usage:\n");  /*there is an error in the input*/
    print_usage();
    printf("aborting...\n");
    return 0;
  }

  /*CHECK THE ALGORITHM*/
  if(!strcmp(argv[4], "lru")){
    algorithm=0;
    lru_array=(int**)malloc(number_of_sets*sizeof(int*));
    assert(lru_array!=NULL);
    for(i=0; i<number_of_sets; i++){
      lru_array[i]=(int*)malloc(number_of_blocks*sizeof(int));
      assert(lru_array[i]!=NULL);
    }
    for(i=0; i<number_of_sets; i++){
      for(j=0; j<number_of_blocks; j++){
	lru_array[i][j]=-1;
      }
    }
    accesses=(int*)malloc(number_of_sets*sizeof(int));
    for(i=0; i<number_of_sets; i++)
      accesses[i]=1;
  }else if(!strcmp(argv[4], "fifo")){ 
   algorithm=1;
    fifo_array=(int*)malloc(number_of_sets*sizeof(int));
    assert(fifo_array!=NULL);
    memset(fifo_array, 0, number_of_sets);
  }else if(!strcmp(argv[4], "random")){
    algorithm=2;
  }else{
    printf("invalid algorithm. Usage:\n");  /*there is an error in the input*/
    print_usage();
    printf("aborting...\n");
    return 0;
  }

  if(number_of_blocks==1)
    algorithm=2;                            /*if there are no more than 1 blocks per set use random algorithm. It is useless
					      but we do it for simplicity*/
  allocate_the_cache(number_of_sets, number_of_blocks, number_of_bytes);  /*allocate the space needed for the memory and initialize it*/
  while(fgets(str, 15, fp)){             /*read a line of the file*/
    printf("\n\n");
    address=strtol(&str[2], NULL, 16);   /*convert the address to decimal*/
    instruction=str[0];                  /*store the instruction before the address (S or L)*/
    printf("address given: %s", &str[2]);
    switch(instruction){
    case 'L':
      if(!load_address(address)){
	/*there is a miss loading this address from the memory. The cpu will now load ti from main memory and store
	  it in cache for future use*/
	printf("load failed. We have a miss\n");
	l_misses++;
	replace[algorithm](address);          /*call replace algorithm to replace the cache line*/
	printf("replaced. Here is the new form:\n");
	print_cache();
      }else{
	printf("Found in cache. We have a hit\n");
	l_hits++;
      }
      if(algorithm==0){                       /*lru algorithm*/
	accessed(address);                    /*call the function to update the usage of the lines*/
      }
      break;
    case 'S':
      continue;
    default:
      assert(0);
    }
  }

  fclose(fp);
  printf("Load hits: %d\n", l_hits);
  printf("Load misses: %d\n", l_misses);
  free_all(algorithm);
  return 1;
}

void free_all(char algorithm){
  int i, j;
  struct cache_line *temp, *next;
  switch(algorithm){
  case 0:                          /*lru*/
  /*free lru array*/
    for(i=0; i<memory.number_of_sets; i++)
      free(lru_array[i]);
    free(lru_array);
    /*free accesses array*/
    free(accesses);
    break;
  case 1:                         /*fifo*/
    /*free fifo array*/
    free(fifo_array);
    break;
  case 2:                         /*random. No operation*/
    break;
  default:
    assert(0);
  }
  /*free sets*/
  for(i=0; i<memory.number_of_sets; i++){
    temp=memory.sets[i];
    for(j=0; j<memory.number_of_blocks; j++){
      next=temp->next;
      free(temp->block);
      free(temp);
      temp=next;
    }
  }
  free(memory.sets);
  return;
}

void accessed(long int address){
  int set=address%memory.number_of_sets;    /*find the set where the line is located*/
  long int tag=address/memory.number_of_sets;    /*find the tag*/
  int i, position;
  struct cache_line *temp;
  temp=memory.sets[set];
  for(i=0; i<memory.number_of_blocks; i++){   /*find the position of the accessed line*/
    if(temp->tag==tag)
      break;
    temp=temp->next;
  }
  position=i;
  lru_array[set][position]=accesses[set];
  accesses[set]++;
  return;
}

/*takes an address and stores in the cache. returns 1 on hit and 0 on miss*/
int store_address(long address){
  return 0;
}

/*takes an address and searches it in the cache. If it finds it returns 1 else returns 0*/
int  load_address(long int address){
  int set=address%memory.number_of_sets;    /*find the set where the line is located*/
  long int tag=address/memory.number_of_sets;    /*find the tag*/
  struct cache_line *temp=memory.sets[set];
  while(temp!=NULL){                        /*access all lines of the set to find the line you want*/
    if(temp->tag==tag)                      /*wanted line found*/
      return temp->valid_bit;               /*return the address*/
    temp=temp->next;
  }
  return 0;                              /*if program is here, tag didnt find. Miss.*/
}

void lru_rep(long int address){
  int set=address%memory.number_of_sets;         /*find the set where the line is located*/
  long int tag=address/memory.number_of_sets;    /*find the tag*/
  int i;
  int min;
  int index_of_min;
  struct cache_line *temp=memory.sets[set];
  /*check if there are invalid lines in cache to write on them*/
  for(i=0; i<memory.number_of_blocks; i++){
    if(temp->valid_bit==0){
      temp->tag=tag;
      temp->valid_bit=1;
      return;
    }
    temp=temp->next;
  }
  /*if you are here there was not empty cache line on this set. you have to overwrite one...*/
  /*You will overwrite according to the lru array. The index with the less number is the least recent used block*/
  min=lru_array[set][0];
  index_of_min=0;
  for(i=1; i<memory.number_of_blocks; i++){
    if(lru_array[set][i]<min){
      min=lru_array[set][i];
      index_of_min=i;
    }
  }
  temp=memory.sets[set];
  for(i=0; i<index_of_min; i++)
    temp=temp->next;
  temp->tag=tag;
  temp->valid_bit=1;
  return;
}

void fifo_rep(long int address){
  int i;
  int set=address%memory.number_of_sets;              /*find the set*/
  long int tag=address/memory.number_of_sets;         /*find the tag*/
  struct cache_line *temp=memory.sets[set];
  int target_block=fifo_array[set];
  for(i=0; i<target_block; i++)
    temp=temp->next;                                  /*go to the target block*/
  temp->tag=tag;
  temp->valid_bit=1;
  fifo_array[set]++;
  if(fifo_array[set]==memory.number_of_blocks)
    fifo_array[set]=0;
  return;
}

void random_rep(long int address){
  int i;
  int random_block=rand()%memory.number_of_blocks;    /*select a random block to store the data*/
  int set=address%memory.number_of_sets;              /*find the set*/
  long int tag=address/memory.number_of_sets;         /*find the tag*/
  struct cache_line *temp=memory.sets[set];
  for(i=0; i<random_block; i++)
    temp=temp->next;
  temp->tag=tag;                                      /*replace the old tag with this.tag*/
  temp->valid_bit=1;
  return;
}

/*allocates the memory for the cache and initializes the blocks*/
void allocate_the_cache(int sets, int blocks, int bytes){
  int i;
  int j;
  struct cache_line *new_line;
  memory.number_of_sets=sets;
  memory.number_of_blocks=blocks;
  
  memory.sets=(struct cache_line**)malloc(sets*sizeof(struct cache_line*));    /*allocate memory for the sets*/
  assert(memory.sets!=NULL);
  for(i=0; i<memory.number_of_sets; i++)
    memory.sets[i]=NULL;                                                       /*initialize with NULL*/
  
  for(i=0; i<memory.number_of_sets; i++){
    for(j=0; j<memory.number_of_blocks; j++){
      new_line=new_block(bytes);                     /*create a new line with a block of <bytes> bytes*/
      new_line->next=memory.sets[i];                 /*link the new line to the hash table*/
      memory.sets[i]=new_line;
    }
  }
  return;
}

/*creates a new cache line and returns it*/
struct cache_line* new_block(int bytes){
  struct cache_line *new;
  new=(struct cache_line*)malloc(sizeof(struct cache_line));   /*allocate memory for the new line*/
  assert(new!=NULL);
  new->valid_bit=0;                                            /*initialize valid bit with 0*/
  new->tag=0;
  new->block=(char*)malloc(bytes);                             /*allocate memory for the block*/
  assert(new->block!=NULL);
  return new;                                                  /*return the new line*/
}

/*prints the cache memory*/
void print_cache(){
  int i, j;
  struct cache_line *temp;
  for(i=0; i<memory.number_of_sets; i++){
    temp=memory.sets[i];
    printf("SET: %d --->", i);
    for(j=0; j<memory.number_of_blocks; j++){
      printf("[valid bit: %d | tag: 0x%lx] ", temp->valid_bit, temp->tag);
      temp=temp->next;
    }
    printf("\n");
  }
}


/*  https://www.geeksforgeeks.org/program-to-find-whether-a-no-is-power-of-two/  */
int is_power_of_two(int number){
  if(number==0)
    return 0;
  while(number!=1){
    if (number%2!=0)
      return 0;
    number=number/2;
  }
  return 1;
}

/*prints a message to the user about the validity of the input*/
void print_usage(void){
  printf("! Number of sets must be power of 2\n");
  printf("! Number of blocks must be power of 2\n");
  printf("! Number of bytes per block must be power of 2\n");
  printf("! Number of bytes per block must be at least 4\n");
  printf("! The algorithm can be: lru, fifo, random\n");
  return;
}
