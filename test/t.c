#include <stdio.h>
#include <stdlib.h>
#include "seats.h"
/* stdin: method parties divs seats v0 v1 ...  -> stdout: seats_won..., tie, cap */
int main(void){
  static result_t r; int m;
  while(scanf("%d %d %d %d",&m,&r.parties,&r.divs,&r.seats)==4){
    r.method=m;
    for(int i=0;i<r.parties;i++) scanf("%u",&r.votes[i]);
    seats_compute(&r);
    for(int i=0;i<r.parties;i++) printf("%d ",r.seats_won[i]);
    printf("| %d %d\n",r.tie,r.cap_hit);
  }
  return 0;
}
