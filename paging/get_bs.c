#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
	if(bsm_tab[bs_id].bs_isheap == 1)
	{
		kprintf("BS %d: Is used by a process as heap\n",bs_id);
		return SYSERR;
	}
	if(npages > 100)
		return SYSERR;
	
	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
	{
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_npages = npages;
	}
	bsm_tab[bs_id].bs_pid[currpid] = 1;
	bsm_tab[bs_id].bs_refCount++;
	//?proctab[currpid].store = bs_id;//shouldn't I change to pBSlist
	ProcBSlist *tVar = getmem(sizeof(ProcBSlist));//getmem
	tVar->store = bs_id;
	//tVar.vhpnpages = retur;
	//tVar.vhpno = virtpage;

	ProcBSlist **ins = &proctab[currpid].pBSlist;
	tVar->next = *ins;
	*ins = tVar;
#ifdef DEBUGuser
	kprintf("BS_MAP\n");
#endif
    return bsm_tab[bs_id].bs_npages;

}


