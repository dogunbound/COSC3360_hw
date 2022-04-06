#include <iostream>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>
//#include <sys/types.h>



using namespace std;

int main(){


int limit = 2;
int sid; // semaphore id
sid = semget(123,2,IPC_CREAT | 0666); //declaring the semaphore 2nd argument determines number of semaphores made in this case 2 are made
//1st arguement is the id 3rd is a flag
struct sembuf sb[2]; // used to control the semaphore somewhat like the 2 size array for a pipe
sb[0].sem_num = 0; // selecting semaphore 0
sb[0].sem_op = -1;//decrement semaphore by 1 (wait) sb[0] will be our wait operation
sb[0].sem_flg = 0;
sb[1].sem_num = 0;
sb[1].sem_op =  1;//increment semaphore by 1 (release) sb[1] will be our release operation
sb[1].sem_flg = 0;

semop(sid,&sb[1],1); // default value of a semaphore is 0 (waiting for a release) so you have to increment it by one like here
// in order for the first process that hit's semaphore 0 to keep running.

int shmid; // shared memory ID
int *bank_account;

shmid = shmget(321,sizeof(bank_account),IPC_CREAT | 0666); // making a shared variable for bank account (this is basically memory allocation).

if(shmid<0){
cout<<"error with shmid"<<endl;
exit(1);
}

bank_account = (int *)shmat(shmid,NULL,0);// shared memory bank account being declared
if (bank_account ==(int *) (-1))
{
cout<<"bank_account error"<<endl;
exit(1);
}
*bank_account = 10; 
// fork hint
int pnum = -1;
int pid;
for(int k =0;k<limit;k++){
pid = fork();
	if(pid ==0){
	pnum = k;
	break;
	}
}

cout<<"ok here"<<endl;
//Re creating bank example
// pnum -1 is the bank account
// pnum 0 is customer 1 depositing 3 $ into the bank account
//pnum 1 is customer 2 depositing $4 into the bank account
switch(pnum)
{
	case(-1):
	sb[0].sem_num = 1;
	sb[0].sem_op = -1;
	cout<<"parent"<<endl;
	semop(sid,&sb[0],1); // wait for both processes 0 and 1 to finish before printing
	semop(sid,&sb[0],1); 
	
	cout<<"parent done"<<endl;
	cout<< *bank_account<<endl;
	break;
	
	case(0):
	sb[0].sem_num = 0;
	sb[1].sem_num = 0;
	cout<< "critical section 0 start"<<endl;
	semop(sid,&sb[0],1);//wait entering critical section of accessing bank_account
	cout<< "critical section 0 in"<<endl;
	//sleep(10);
	*bank_account = *bank_account + 3;
	semop(sid,&sb[1],1);
	cout<< "critical section 0 end"<<endl;
	sb[1].sem_num = 1;
	semop(sid,&sb[1],1); // wait for both processes 0 and 1 to finish before printing
	cout<<"finish 0"<<endl;
	break;
	
	case(1):
	sb[0].sem_num = 0;
	sb[1].sem_num = 0;
	//sleep(2);
	cout<< "critical section 1 start"<<endl;
	semop(sid,&sb[0],1);//wait entering critical section of accessing bank_account
	cout<< "critical section 1 in"<<endl;
	*bank_account = *bank_account + 4;
	semop(sid,&sb[1],1);
	cout<< "critical section 1 end"<<endl;
	
	sb[1].sem_num = 1;
	semop(sid,&sb[1],1); // wait for both processes 0 and 1 to finish before printing
	cout<<"finish 1"<<endl;
	break;	
}
sleep(1); // if the semaphores are deallocated before they are finished being used will cause issues.
shmdt(bank_account);
shmctl(shmid,0,IPC_RMID);
semctl(sid,0,IPC_RMID);
/* use these if you don't use the above commands to remove semaphores and shared memory courtesy of pavan
ipcs -q | awk '{ print "ipcrm -q "$2}' | sh > /dev/null 2>&1; ipcs -m | awk '{ print "ipcrm -m "$2}' | sh > /dev/null 2>&1; ipcs -s | awk '{ print "ipcrm -s "$2}' | sh > /dev/null 2>&1;
*/
return 0;
}
