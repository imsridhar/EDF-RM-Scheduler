/* Program to implement the discrete event simulation of the rate monotonic and earliest deadline first scheduling 
   the events are 
   1.When the process enters the cpu
   2. When the process is preempted
   3. When the process is resumed
   4. When the cpu is in idle state
   5. Whe the process is completed
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include<bits/stdc++.h>
using namespace std;
typedef struct {  // structure for process as well as events
	int pid;		
	int period;
	int processTime;
	int k;			// number of times the process is to be executed
	int deadline;
	int isMissedDeadline;	// whether the deadline is missed for each execution of the process
	int missedDeadline;	// whether the deadline is missed for the whole execution of the process
	int waitTime;	
	int startTime;	// Starting time of the process
	int timeDelta;	// Time quanta remaining
	int mx;	//max response time for a job of a task
	int mn;	// min response time fora job of a task 
} process;
// apne case me period hi deadline he
process * createProcess(int pi,int t,int p,int k){  // Method used to create the process 
	process *proc = new process;
		proc->pid=pi;
		proc->processTime=t;					
		proc->period=p;
		proc->k=k;
		proc->deadline=proc->period;
		proc->startTime=0;
		proc->timeDelta=proc->processTime;
		// std::cout<<proc->timeDelta<<"\n";
		proc->isMissedDeadline=0;
		proc->missedDeadline=0;
		proc->waitTime=0;
		proc->mx =0;
		proc->mn =1000000000;
		return proc;
}

bool cmpPtrtoNodeforedf(process *a,process *b){		// Method used to compare while sorting the queue as per priority
	return a->deadline<b->deadline;
}



class CPU{
	// CPU class which represents an actual like cpu that holds processes has clock 
	public:
		process * p;
		int ticks;
		int num_preempted;
		std::fstream  f;
		CPU(std::string s){		// Constructor 
			ticks=0;
			num_preempted=0;
			readyCumEventqueue.reserve(52);
			f.open(s,std::ios::out);
		}
		~CPU(){				// Destructor
			f.close();
			// std::cout<<"CPU stopped\n";
		}
		int processIt(process *);			// To process the process onto the CPU
		int preempt(int);					// To preempt the process
		int complete();						// To complete the process once it's processing time ends
		int edfprocessing();
		void updateProcessInCpu(process *);				// To do earliest deadline first processing in the cpu
		int cpuIdle(int);					// To make the rm cpu idle
		int cpuresume(process *);			// TO resume the preempted process
		void edfschedule();					// To link edfscheduler to the cpu
		int isPreempted(std::vector<process *>);					
		int isschedulable(std::vector<process *>);	// To check if any process present to schedule
		process * getEvent(std::vector<process *>);	// 
		std::vector<process *> readyCumEventqueue;

};

int CPU::isschedulable(std::vector<process *> qu){ // Method to check if any process is available to schedule
	int count=0;
	for (int i=0;i<qu.size();i++){
		if (qu[i]->k<=0){  // checking if the process is available to execute
			count+=1;
		}
//		std::cout<<"the k value is "<<qu[i]->k<<"\n";
	}
//	std::cout<<"the count is "<<count<<" the size of qu is "<<qu.size()<<"\n";
	if (count==qu.size()) return 0;
	else return 1;
}
void CPU::edfschedule(){
	// Method to schedule the processes according to earliest deadline first scheduling
	std::sort(readyCumEventqueue.begin(),readyCumEventqueue.end(),cmpPtrtoNodeforedf);	
	int flag;
	process * someOtherProcess;
	processIt(readyCumEventqueue[0]); // First schedule the first process
	flag=edfprocessing();
	
	while (isschedulable(readyCumEventqueue)){

		someOtherProcess=getEvent(readyCumEventqueue);  // If any event present at the time then get the process pointer
		
		if (someOtherProcess){
		
			if (someOtherProcess->timeDelta!=someOtherProcess->processTime)cpuresume(someOtherProcess); // if process was preempted then resume it
			else
			{
				if (someOtherProcess->startTime==0) someOtherProcess->waitTime=ticks;	// if the process was executing the first time then calulate the waittime
				processIt(someOtherProcess);
			} 
			flag=edfprocessing();
		}
		else cpuIdle(1); 
	}
}

process * CPU::getEvent(std::vector<process*> qu){
	// The function is checking if there is any event to dispatch to the cpu
	for (int i=0;i<qu.size();i++){
		if ((qu[i]->startTime==ticks || qu[i]->startTime <ticks) && qu[i]->k>0) return qu[i]; 
		 
	}
	
	return 0;
}

int CPU::isPreempted(std::vector<process *> qu){ // Method is checking if the process will be preempted or not
	//std::cout<<"Entered the preemt check function\n";
	process * temp=getEvent(qu);
	if ((temp!=0)){
		if (p->deadline > temp->deadline){
			preempt(temp->pid);
			// ticks++; // comment it if you dont't want to include context switch time
			return 1;
		}
	}
	return 0;

}

void CPU::updateProcessInCpu(process *proc){  // TO update the process in the cpu once it completes it's execution
	proc->startTime+=proc->period; // updating the start time of the process
	if (proc->isMissedDeadline){while (proc->startTime<ticks)proc->startTime+=proc->period;}
	proc->deadline=proc->startTime+proc->period;  // updating the deadline
	proc->timeDelta=proc->processTime; // updating the reamining quanta
	// std::cout<<"new deadline=="<<proc->deadline<<" new startime=="<<proc->startTime<<"\n";
	p->k-=1;
	p->isMissedDeadline=0;		// Once completed 
	p=NULL;

}

int CPU::edfprocessing(){ 
	while (!isPreempted(readyCumEventqueue)){  // While the process is not preempted then continue
		ticks++;
		p->timeDelta-=1;
		if (p->timeDelta==0){ // if process completed it's execution
			// std::cout<<"Completed\n";
			if (p->deadline < ticks){
				// std::cout<<"deadline Missed \n"; 
				p->isMissedDeadline=1;
				p->missedDeadline+=1;
				}
			// if(p->deadline - ticks<0){
				// p->mx = max(p->mx,ticks - p->deadline + p->period);
				// p->mn = min(p->mn,ticks - p->deadline + p->period);	
			// }
			// else{
				// cout << p->deadline << " " << ticks << endl;
				if(p->deadline >= ticks){
				p->mx = max(p->mx,p->period - ( p->deadline - ticks ));
				p->mn = min(p->mn,p->period - ( p->deadline - ticks ));}
			// }
			complete();     // log the event
			// ticks++;   // comment it if you don't want to include context switch time
			updateProcessInCpu(p);
			sort(readyCumEventqueue.begin(),readyCumEventqueue.end(),cmpPtrtoNodeforedf); // rearrange the queue according to the deadline
			return 1;
		}

	}
	return 0;

}

int CPU::processIt(process * pro){// TO start the processing of the process
	// std::cout<<"Process "<<pro->pid<<" Deadline:"<<pro->deadline<<" processTime:"<<pro->processTime<<" period:"<<pro->period<<" entered the CPU at time "<<ticks<<"\n";
	f<<"Process "<<pro->pid<<" Deadline:"<<pro->deadline<<" processTime:"<<pro->processTime<<" period:"<<pro->period<<" entered the CPU at time "<<ticks<<"\n";
	p=pro;
	return 1;
}
int CPU::cpuIdle(int sched){ // To indicate the idle cpu
	std::vector<process*> temp=readyCumEventqueue;
	while (!getEvent(temp)) ticks++;
	// std::cout<<"CPU is in idle state till time = "<<ticks<<"\n";
	f<<"CPU is in idle state till time = "<<ticks<<"\n";

	return 1;
}

int CPU::preempt(int procid){	// To preempt the process from the cpu
	// std::cout<<"Process "<<p->pid<<" preempted by "<<procid<<" at time "<<ticks<<" with remaining process time "<<p->timeDelta<<"\n"; 
	f<<"Process "<<p->pid<<" preempted by "<<procid<<" at time "<<ticks<<" with remaining process time "<<p->timeDelta<<"\n"; 
	num_preempted+=1;
	return 1;
}
int CPU::complete(){ // To signal completion of the process
	// std::cout<<"Process "<<p->pid<< " completed its execution at time "<<ticks<<"\n";
	if (p->isMissedDeadline==1)	f<<"Process "<<p->pid<< " completed its execution at time "<<ticks<<" and missed deadline\n";
	else f<<"Process "<<p->pid<< " completed its execution at time "<<ticks<<"\n";

	return 1;
}
int CPU::cpuresume(process * proc){ // To log the resume of the process
	// std::cout<<"Process  "<<proc->pid<<" resumes its execution at time "<<ticks<<"\n";
	f<<"Process "<<proc->pid<<" resumes its execution at time "<<ticks<<"\n";

	p=proc; 
	return 1;
}
int main(){
	
	std::fstream ifile,statsFile2;
	// ifile.open("inp-params.txt",std::ios::in);
	std::string log2="EDF-Log.txt";
	statsFile2.open("EDF-stats.txt",std::ios::out);

	int n;
	// ifile>>n;
	int totalprocesses=0;
//	int n=2;
	//period, execution, deadline
	int pid,t,p,k;
	CPU c2(log2);
	srand(time(0));
	cout << "EDf" << endl;
	// Number of preemptions VS number of tasks
	cout << "Average number of preemptions VS Number of tasks" << endl;
	for(int q=1;q<=15;q++)
	{
		int tot=0;
		for(int jj=0;jj<200;jj++)
		{
			c2.readyCumEventqueue.clear();
			for(int i=1;i<=4*q;i++){
				//std::cin>>pid>>t>>p>>k;
				pid = i;
				t = rand()%7+1;
				p = (i+2)*(q+1);
				k = 30;
				process *procForedf=createProcess(pid,t,p,k);
				c2.readyCumEventqueue.push_back(procForedf);		
				
			}
			c2.edfschedule();
			// cout << "Number of times process preempted " << c2.num_preempted << "\n";
			tot+=c2.num_preempted;
			c2.num_preempted =0 ;
		}
		cout << tot/200 << ",";//to calculate tasks average

	}
	cout << endl;
	// Preemptions VS load
	cout << "Average number of preemptions VS Load" << endl;
	for(float q=0.5;q<=0.96;q+=0.05)
	{
		int qwe=0;
		
			c2.readyCumEventqueue.clear();
			for(int i=1;i<=10;i++){
				pid = i;
				t = rand()%7+1;
				p  = rand()%100+1;
				t = (q-0.35)*p+1;

				k = 20;
				process *procForedf=createProcess(pid,t,p,k);
				c2.readyCumEventqueue.push_back(procForedf);		
			}
			c2.edfschedule();
			// cout << "Number of times process preempted " << c2.num_preempted << "\n";
			qwe+=c2.num_preempted;
			c2.num_preempted =0 ;
		cout << qwe  << ',';

	}
	cout << endl;
	
	// Normalized ARJ VS Task number
	cout << "Normalized ARJ VS Task number" << endl; 
	c2.readyCumEventqueue.clear();
	for(int i=1;i<=10;i++){
		pid = i;
		p = (i+2)*(5);
		t = 0.5*p;
		k = 10;
		process *procForedf=createProcess(pid,t,p,k);
		c2.readyCumEventqueue.push_back(procForedf);		
	}
	c2.edfschedule();
	for(auto x:c2.readyCumEventqueue){
		cout << ((float)x->mx - (float)x->mn)/(float)x->period << ',';
	}
	cout << endl;
// 	To find the statistics of the scheduler 
	
	int numDeadlineMissed=0;
	float avgWaitTime=0;

// For edf scheduler

	for (int i=0;i<c2.readyCumEventqueue.size();i++){
		if (c2.readyCumEventqueue[i]->missedDeadline>=1) numDeadlineMissed+=c2.readyCumEventqueue[i]->missedDeadline;
		avgWaitTime+=c2.readyCumEventqueue[i]->waitTime;
	}
	avgWaitTime/=c2.readyCumEventqueue.size();
	statsFile2<<"The number of processes that entered the cpu "<<totalprocesses<<"\n";
	statsFile2<<"Number of processes that missed Deadline = "<<numDeadlineMissed<<"\n";
	statsFile2<<"The average waiting time = "<<avgWaitTime<<"\n";
	statsFile2<<"Number of processes that completed without missing deadline "<<totalprocesses-numDeadlineMissed<<"\n";
	ifile.close();
	statsFile2.close();
	// cout << "Number of times process preempted " << c2.num_preempted << "\n";
	return 0;
}
// 2
// 1 2 5 7
// 2 4 7 5