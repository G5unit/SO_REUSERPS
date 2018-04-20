/*  sock_reuserps.c

  Faruk Grozdanic github/g5unit
  
  Released under GPLv3.0
  
*/

/*
  add sk->reuserps as int variable to linux kernel struct sock in sock.h
*/

 #include "sock_reuserps.h"

 #define SO_REUSERPS 1

#define SORPS_BITMASK_MAXLENGTH 128
#define SORPS_BUCKET_MAXSIZE 128

typedef struct sorps_fdentry_t {
  int count;
  struct socket * fd;
} sorps_fdentry;

typedef struct sorps_tableEntry_t {
  int next;
  int bucket_length;
  struct sorps_fdentry_t bucket[SORPS_BUCKET_MAXSIZE];
} sorps_tableEntry;



typedef struct sorps_listentry sorps_table_t[SORPS_BITMASK_MAXLENGTH] sorps_table;

int sorps_add_listener(struct sock* sk, struct socket* fd, int cpuid){
  sorps_table sorpstpt = *(sk->reuserps_table);
  sorps_fdentry* sorpsfdept = (sorps_fdentry *)malloc(sizeof(sorps_fdentry_t));
  sorps_fdentry sorpsfde;
  if(!sorpsfdept) {
    return 1;
  }
  sorpsfde.fd = fd;
  if(unlikely(sorpstpt[cpuid].bucket_length == SORPS_BUCKET_MAXSIZE)) {
    return 1;
  }
  sorpstpt[cpuid].bucket[sorpstpt[cpuid].bucket_length] = sorpsfde;
  sorpstpt[cpuid].bucket_length ++;
  return 0;
}

int sorps_decrement_listener(struct sock* sk, struct socket* fd, int cpuid){
  sorps_table sorpstpt = *(sk->reuserps_table);
  for(int i=0;i<sorpstpt[cpuid].bucket_length;i++) {
    if(sorpstpt[cpuid].bucket[i].fd == fd) {
      sorpstpt[cpuid].bucket[i].count --;
      /* sanity check that we are not negative, something went out of sync */
      if(unlikely(sorpstpt[cpuid].bucket[i].count < 0)) {
        sorpstpt[cpuid].bucket[i].count = 0;
      }
      return 0;
    }
  }
  return 1;
}

int sorps_remove_listener(struct socket* fd, int cpuid) {

  return 0; //no match found so nothing to remove
}

struct socket* sorps_lookup_listener(struct sock* sk, int cpuid) {
  sorps_table sorpstpt = *(sk->reuserps_table);
  int sorpindx = sorpstpt[cpuid].next;
  if(sorpstpt[cpuid].next) {

    struct socket* fd = sorpstpt[cpuid].next;
    /* call function to assign next. We need to support multiple routing schemes */

    /* return fd */
    return fd;
  }

}

int _sorps_init_table(struct sock* sk) {
  sorps_table* sorpstpt = (sorpstpt *)malloc(sizeof(sorps_table));
  if(!sorpstpt) {
    return 1;
  }
  sk->reuserps_table = sorpstpt;
  return 0;

}
