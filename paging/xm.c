/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  /* sanity check ! */
  STATWORD ps;	
  disable(ps);
  if ( (virtpage < 4096) || ( source < 0 ) || ( source > MAX_ID) ||(npages < 1) || ( npages > 100)){
 	restore(ps);
	return SYSERR;
  }
	if(bsm_tab[source].bs_pid[currpid] != 1) return SYSERR;//Backing store not mapped to process
	bsm_tab[source].bs_vpno[currpid] = virtpage;


	ProcBSlist *ins = proctab[currpid].pBSlist;

	while(ins!=NULL && ins->store!=source){
		ins = ins->next;
	}
	if(ins==NULL) return SYSERR;// Backing store not mapped to process
	ins->vhpnpages = bsm_tab[source].bs_npages;
	ins->vhpno = virtpage;
#ifdef DEBUGuser
ProcBSlist *t = proctab[currpid].pBSlist;
while(t!=NULL){
kprintf("%x  Store :%d   VHPNO: %d NPAGES: %d\n",t,t->store,t->vhpno,t->vhpnpages);
t = t->next;
}
kprintf("\n");
#endif
	  restore(ps);
	  return OK;
}


/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage )
{
	int i,tmp,j,k;
	//int npages = proctab[currpid].vhpnpages;
	//free_frm_list *freefrm,*freefrm1;
    //used_frm_list *usedfrm,*prevfrm,*usedfrm1;
	//int c=0;
	int bs_id;
	int vhpno;
	int tpg;
	STATWORD ps;
    disable(ps);	
  /* sanity check ! */
  if ( (virtpage < 4096) ){ 
 	restore(ps);
	kprintf("xmunmap call error: virtpage (%d) invalid! \n", virtpage);
	return SYSERR;
  }
	if(proctab[currpid].vhpno<=virtpage && (proctab[currpid].vhpno+proctab[currpid].vhpnpages) > virtpage){//Release heap
		bs_id = proctab[currpid].store;
		vhpno = proctab[currpid].vhpno;
		tpg = proctab[currpid].vhpnpages;
		{//REPEATED in ELSE
			used_frm_list **t_frm;
			t_frm = &usedhead;
			while(*t_frm !=NULL){
				if(frm_tab[(*t_frm)->frameno].fr_pid  = currpid){
					if(vhpno<=frm_tab[(*t_frm)->frameno].fr_vpno && vhpno+tpg>frm_tab[(*t_frm)->frameno].fr_vpno){

						free_frm((*t_frm)->frameno);
						free_frm_list* t1_frm = *t_frm ;
						*t_frm  = (*t_frm )->next;
						t1_frm->next = freehead;
						freehead = t1_frm;
						continue;
					}
				}
				t_frm = &(*t_frm)->next;
			}	
				bsm_tab[bs_id].bs_vpno[currpid] = -1;
				proctab[currpid].vhpno = -1;//OR t->vhpno = -1
		}
	}else{//File
		ProcBSlist *t = proctab[currpid].pBSlist;
		while(t != NULL){
			if(t->vhpno<=virtpage && (t->vhpno + t->vhpnpages)>virtpage){
				bs_id = t->store;
				vhpno  = t->vhpno;
				tpg = t->vhpnpages;
				{//REPEATED in IF
					used_frm_list **t_frm;
					t_frm = &usedhead;
					while(*t_frm !=NULL){
						if(frm_tab[(*t_frm)->frameno].fr_pid  = currpid){
							if(vhpno<=frm_tab[(*t_frm)->frameno].fr_vpno && vhpno+tpg>frm_tab[(*t_frm)->frameno].fr_vpno){

								free_frm((*t_frm)->frameno);
								free_frm_list* t1_frm = *t_frm ;
								*t_frm  = (*t_frm )->next;
								t1_frm->next = freehead;
								freehead = t1_frm;
								continue;
							}
						}
						t_frm = &(*t_frm)->next;
					}	
						bsm_tab[bs_id].bs_vpno[currpid] = -1;
						t->vhpno = -1;
				}
				break;
			}
			t = t->next;
		}

		if(t == NULL){restore(ps); return SYSERR;}
	}
	/*for(i=0;i<NFRAMES;i++){
		if(frm_tab[i].fr_pid==currpid){
			if(vhpno<virtpage&&vhpno+tpg>virtpage){
			free_frm(i);
			}
		}
	}*/

}
