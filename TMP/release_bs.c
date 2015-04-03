#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED) return SYSERR;
	if(bsm_tab[bs_id].bs_pid[currpid] == -1) return SYSERR;
	if(	!(--bsm_tab[bs_id].bs_refCount)) {
		if(bsm_tab[bs_id].bs_isheap == 1) bsm_tab[bs_id].bs_isheap = 0;
		bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
		bsm_tab[bs_id].bs_npages = 0;
	}
	bsm_tab[bs_id].bs_pid[currpid] = -1;
//	proctab[currpid].store = -1;//shouldn't I change to pBSlist
	ProcBSlist **ins = &proctab[currpid].pBSlist;
	if(*ins==NULL) return SYSERR;
	while((*ins)->store!=bs_id){
		(*ins) = &(*ins)->next;
	}
	ProcBSlist *t = *ins;
	*ins = (*ins)->next;
	freemem(t,sizeof(ProcBSlist));
   return OK;

}

