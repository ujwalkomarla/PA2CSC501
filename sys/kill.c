/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	used_frm_list **t_frm;
		t_frm = &usedhead;
		while(*t_frm != NULL){
			if(frm_tab[(*t_frm )->frameno].fr_pid = currpid){
				if(frm_tab[(*t_frm )->frameno].fr_type != FR_PAGE){
					frm_tab[(*t_frm )->frameno].fr_status = FRM_UNMAPPED;
					frm_tab[(*t_frm )->frameno].fr_vpno = -1;
					frm_tab[(*t_frm )->frameno].fr_pid = -1;
					frm_tab[(*t_frm )->frameno].fr_type = -1;
				}else{
					free_frm((*t_frm )->frameno);
				}
				//REMOVE from usedhead and insert in freehead
				free_frm_list* t1_frm = *t_frm ;
				*t_frm  = (*t_frm )->next;
				t1_frm->next = freehead;
				freehead = t1_frm;
				continue;
			}
			t_frm = &(*t_frm)->next;
		}
int i;
for(i=0;i<16;i++){
	if(bsm_tab[i].bs_status ==BSM_MAPPED && bsm_tab[i].bs_pid[currpid] == 1){
		if(bsm_tab[i].bs_isheap == 1){//HEAP
			bsm_tab[i].bs_status = BSM_UNMAPPED;
			bsm_tab[i].bs_pid[pid] = -1;
			bsm_tab[i].bs_refCount = 0;
		}else{//FILE
		
			bsm_tab[i].bs_pid[pid] = -1;
			bsm_tab[i].bs_refCount--;
			if(bsm_tab[i].bs_refCount == 0){
				bsm_tab[i].bs_status = BSM_UNMAPPED;
			}
		}

	}
}
//RELEASE MEMORY in PROCTAB
ProcBSlist *t = NULL;
ProcBSlist **ins = &proctab[currpid].pBSlist;
while(*ins!=NULL){
	t = (*ins)->next;
	freemem(*ins,sizeof(ProcBSlist));
	*ins = t;
}
	restore(ps);
	return(OK);
}
