/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	int i,*avail,offset,bs_id;
	STATWORD ps;
	
	pt_t *pt; 
	virt_addr_t tmp;
	
	pd_t *pd = read_cr3();
	u_long addr = read_cr2();
	
	tmp.pd_offset = (addr >> 22);
	tmp.pt_offset = (addr >> 12) & 0x3FF;
	tmp.pg_offset = (addr & 0xFFF);
	disable(ps);
	//Shouldn't I first check if that region(i.e. LEGAL) is mapped in proctab
	
	int searchPg = addr/NBPG;
	if(proctab[currpid].vhpno < searchPg && (proctab[currpid].vhpno + proctab[currpid].vhpnpages) >searchPg){//HEAP
		bs_id = proctab[currpid].store;
		offset = (addr >>12)- proctab[currpid].vhpno;
	}else{//FILES
		ProcBSlist *t = proctab[currpid].pBSlist;
		while(t != NULL){
			if(t->vhpno<searchPg && (t->vhpno + t->vhpnpages)>searchPg){
				bs_id = t->store;
				offset = (addr>>12) - t->vhpno;
				break;
			}
			t = t->next;
		}

		if(t == NULL){restore(ps); return SYSERR;}
	}

	if(pd[tmp.pd_offset].pd_pres == 0){
		int *avail;
		get_frm(avail);
		frm_tab[(*avail)].fr_status = FRM_MAPPED;
		frm_tab[(*avail)].fr_pid = currpid;
		frm_tab[(*avail)].fr_type = FR_TBL;
		frm_tab[(*avail)].fr_vpno = -1;
		
		pd[tmp.pd_offset].pd_pres = 1;
		pd[tmp.pd_offset].pd_write = 1;
		pd[tmp.pd_offset].pd_base = (*avail)+FRAME0;
		
		
		
	}		


	pt = (pt_t*)((pd[tmp.pd_offset].pd_base) * NBPG);
	get_frm(avail);
	frm_tab[(*avail)].fr_status = FRM_MAPPED;
	//MAP all process using the given backing store.
		frm_tab[(*avail)].fr_pid = currpid;
		frm_tab[(*avail)].fr_vpno = addr/NBPG;
	frm_tab[(*avail)].fr_type = FR_PAGE;
	pt[tmp.pt_offset].pt_pres = 1;
	pt[tmp.pt_offset].pt_write = 1;
	pt[tmp.pd_offset].pt_base = (*avail)+FRAME0;
	read_bs((*avail+FRAME0)*NBPG,bs_id,offset);
	restore(ps);
	return OK;


}		
