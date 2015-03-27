/* vcreate.c - create, newpid */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

LOCAL int newpid();

/*-------------------------------------------------------------------------------
 *  vcreate  -  create a process to start running a procedure with Virtual Memory
 *-------------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int hsize;			/*virtual(private) heap size in npages*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	//create : Normal process creation which includes page directory base setup
	//Initialize Virtual heap backing store in this function (store)
	//Initialize Virtual heap size in this function (vhpnpages)
	//Initialize Virtual Memlist in this function (vmemlist)
	//Initialize Virtual page number in this function(vhpno)
	STATWORD 	ps;
	disable(ps);
	int pid;	
	pid = create(procaddr,ssize,priority,name,nargs,args);
	if(pid==SYSERR){
		restore(ps);
		return SYSERR;		
	}

	int i;
	for(i=0;i<16;i++){//Number of Backing stores : 16
		if(bsm_tab[i].bs_status == BSM_UNMAPPED){
			bsm_tab[i].bs_status = BSM_MAPPED;
			break;
		}
	}
	bsm_tab[i].bs_npages = hsize;
	bsm_tab[i].bs_isheap = 1;
	bsm_tab[i].bs_pid[pid] = 1;
	bsm_tab[i].bs_refCount = 1;
	bsm_tab[i].bs_vpno[pid]=4096;
	//privBS = i;
	/*if(i<TOTAL_BSM) privBS = i;
	else{
		proctab[pid].pstate = PRFREE;
		//free the pdbr frame.		
		restore(ps);
		return SYSERR;	
	}*/
	proctab[pid].store = i;
	proctab[pid].vhpno = 4096;
  	proctab[pid].vhpnpages = hsize;
	proctab[pid].vmemlist = getmem(sizeof(struct mblock));
	proctab[pid].vmemlist->mnext = (struct mblock*)(4096*NBPG);
    proctab[pid].vmemlist->mlen = 0;
	proctab[pid].vmemlist->mnext->mnext = NULL;
	proctab[pid].vmemlist->mnext->mlen = (hsizes)*NBPG;
	
    

	restore(ps);
	return pid;
}
