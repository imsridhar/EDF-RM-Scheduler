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

typedef struct {   // structure for process as well as events
	int pid;		
	int period;
	int processTime;
	int k;			// number of times the process is to be executed
	int deadline;
	int isMissedDeadline;
	int missedDeadline;	// whether the deadline is missed or not
	int waitTime;	
	int startTime;	// Starting time of the process
	int timeDelta;	// Time quanta remaining
	int mx;	//max response time for a job of a task
	int mn;	// min response time fora job of a task 
} process;

process * createProcess(int pi,int t,int p,int k){ //Method to create the process
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
bool cmpPtrtoNodeforrm(process *a,process *b){  // Used to sort the vector queue according to the priority
	return a->period<b->period;
	
}
class CPU{
	// CPU class which represents an actual like cpu that holds processes has clock 
	public:
		process * p;
		int ticks;
		int num_preempted;
		std::fstream  f;
		CPU(std::string s){
			ticks=0;
			num_preempted = 0;
			readyqueueCumEventqueue.reserve(52);
			f.open(s,std::ios::out);
		}
		~CPU(){
			f.close();
			// std::cout<<"CPU stopped\n";
		}
		int processIt(process *);			// To process the process onto the CPU
		int preempt(int);					// To preempt the process
		int complete();						// To complete the process once it's processing time ends
		int rmprocessing();					// To do rate monotonic processing in the cpu
		void updateProcessInCpu(process *);				// To do earliest deadline first processing in the cpu
		int cpuIdle(int);					// To make the rm cpu idle
		int cpuresume(process *);			// TO resume the preempted process
		void rmschedule();
		int isPreempted(int,std::vector<process *>);					
		int isschedulable(std::vector<process *>);	// To check if any process present to schedule
		process * getEvent(std::vector<process *>);	// 
		
		std::vector<process *> readyqueueCumEventqueue;

};

int CPU::isschedulable(std::vector<process *> qu){ // Method to check if the process is schedulable
	int count=0;
	for (int i=0;i<qu.size();i++){
		if (qu[i]->k<=0){  // checjing if the process is available to execute
			count+=1;
		}
	}
//	std::cout<<"the count is "<<count<<" the size of qu is "<<qu.size()<<"\n";
	if (count==qu.size()) return 0;
	else return 1;
}

void CPU::rmschedule(){
	// Method to schedule the processes using rate monotonic scheduling
	std::sort(readyqueueCumEventqueue.begin(),readyqueueCumEventqueue.end(),cmpPtrtoNodeforrm);
	int flag;
	process * someOtherProcess;
	processIt(readyqueueCumEventqueue[0]);
	// std::cout<<" PRocess entered\n";
	flag=rmprocessing();
	
	while (isschedulable(readyqueueCumEventqueue)){
		someOtherProcess=getEvent(readyqueueCumEventqueue);   // someOtherProcess contains the schedulable process
		if (someOtherProcess){
			if (someOtherProcess->timeDelta!=someOtherProcess->processTime)cpuresume(someOtherProcess); // If that process was preemted then resume it's execution
			else{
				if (someOtherProcess->startTime==0) someOtherProcess->waitTime=ticks;	
				processIt(someOtherProcess);
			} 
			flag=rmprocessing();
		}
		else cpuIdle(0); // if no schedulable process then cpu in idle state
	}

}
process * CPU::getEvent(std::vector<process*> qu){
	// The function is checking if there is any event to dispatch to the cpu
	for (int i=0;i<qu.size();i++){
		if ((qu[i]->startTime==ticks || qu[i]->startTime <ticks) && qu[i]->k>0) return qu[i]; 
		
	}
	
	return 0;
}

int CPU::isPreempted(int sched,std::vector<process *> qu){ // Method to check if process is preempted
//	std::cout<<"Entered the preemt check function\n";
	process * temp=getEvent(qu);
	if ((temp!=0)){
		if (p->period > temp->period){
			preempt(temp->pid);
			// ticks++; // comment it if you don't want to include context switch time
			return 1;
		}
	}
	return 0;

}
void CPU::updateProcessInCpu(process *proc){  // TO update the process in the cpu once it completes it's execution
	proc->startTime+=proc->period;
	if (proc->isMissedDeadline){while (proc->startTime<ticks)proc->startTime+=proc->period;}
	proc->deadline=proc->startTime+proc->period;
	proc->timeDelta=proc->processTime;
	// std::cout<<"new deadline=="<<proc->deadline<<" new startime=="<<proc->startTime<<"\n";
	p->k-=1;
	p->isMissedDeadline=0;		// Once completed 
	p=NULL;

}
int CPU::rmprocessing(){  // Method to do the processing in the cpu
	while (!isPreempted(0,readyqueueCumEventqueue)){  // While the process is not preempted then continue
		ticks++;
		p->timeDelta-=1;
		if (p->timeDelta==0){ // if the process completes it's execution
			if (p->deadline < ticks){
				// std::cout<<"deadline Missed \n"; 
				p->isMissedDeadline=1;
				p->missedDeadline+=1;}
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
			complete();
			// ticks++;  // Comment it if you don't want to include context switch time
			updateProcessInCpu(p);
			std::sort(readyqueueCumEventqueue.begin(),readyqueueCumEventqueue.end(),cmpPtrtoNodeforrm);
			return 1;
		}

	}
	return 0;

}

int CPU::processIt(process * pro){// TO log the processing of the process
	// std::cout<<"Process "<<pro->pid<<" Deadline:"<<pro->deadline<<" processTime:"<<pro->processTime<<" period:"<<pro->period<<" entered the CPU at time "<<ticks<<"\n";
	f<<"Process "<<pro->pid<<" Deadline:"<<pro->deadline<<" processTime:"<<pro->processTime<<" period:"<<pro->period<<" entered the CPU at time "<<ticks<<"\n";

	p=pro;
	return 1;
}
int CPU::cpuIdle(int sched){ // To make the cpu go in idle state
	std::vector<process *> temp;
	temp=readyqueueCumEventqueue;

	while (!getEvent(temp)) ticks++;
	// std::cout<<"CPU is in idle state till time = "<<ticks<<"\n";
	f<<"CPU is in idle state till time = "<<ticks<<"\n";

	return 1;
}

int CPU::preempt(int procid){	// To log preemption og the process from the cpu
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
int CPU::cpuresume(process * proc){ // to signal the scheduler that process is resumed
	// std::cout<<"Process  "<<proc->pid<<" resumes its execution at time "<<ticks<<"\n";
	f<<"Process  "<<proc->pid<<" resumes its execution at time "<<ticks<<"\n";

	p=proc; 
	return 1;
}

int main(){
	
	std::fstream ifile,statsFile1;
	ifile.open("inp-params.txt",std::ios::in);
	std::string log1="RM-Log.txt";
	statsFile1.open("RM-stats.txt",std::ios::out);

	int n;
	ifile>>n;

	int totalprocesses=0;
	int pid,t,p,k;
	CPU c1(log1);
	srand(time(0));
	cout << "RM" << endl;

	// Number of preemptions VS number of tasks
	cout << "Average number of preemptions VS Number of tasks" << endl;
	for(int q=1;q<=15;q++)
	{
		int qwe=0;
		for(int jj=0;jj<200;jj++)
		{
			c1.readyqueueCumEventqueue.clear();
			for(int i=1;i<=4*q;i++){
				pid = i;
				t = rand()%7+1;
				p = (i+2)*(q+1);
				k = 30;
				// cout << i << " " << t << " " << p << " " << k << endl;
				process *procForedf=createProcess(pid,t,p,k);
				c1.readyqueueCumEventqueue.push_back(procForedf);		
				
			}
			c1.rmschedule();
			// cout << "Number of times process preempted " << c2.num_preempted << "\n";
			qwe+=c1.num_preempted;
			c1.num_preempted =0 ;
		}
		cout << qwe/200 << ",";

	}
	cout << endl;
	// Preemptions VS load
	cout << "Average number of preemptions VS Load" << endl;
	for(float q=0.5;q<=0.96;q+=0.05)
	{
		int qwe=0;
			c1.readyqueueCumEventqueue.clear();
			for(int i=1;i<=10;i++){
				pid = i;
				p = (i+2)*(100);
				t = q*p;
				k = 10;
				// cout << i << " " << t << " " << p << " " << k << endl;
				process *procForedf=createProcess(pid,t,p,k);
				c1.readyqueueCumEventqueue.push_back(procForedf);		
			}
			c1.rmschedule();
			// cout << "Number of times process preempted " << c2.num_preempted << "\n";
			qwe+=c1.num_preempted;
			c1.num_preempted =0 ;
		cout << qwe  << ',';
	}
	cout << endl;
	c1.readyqueueCumEventqueue.clear();
	for(int i=1;i<=10;i++){
		pid = i;
		// p = (i+2)*(3);
		p = rand()%50+1;
		t = 0.5*p;
		k = 10;
		// cout << i << " " << t << " " << p << " " << k << endl;
		process *procForedf=createProcess(pid,t,p,k);
		c1.readyqueueCumEventqueue.push_back(procForedf);		

	}
	c1.rmschedule();
	for(auto x:c1.readyqueueCumEventqueue){
		// cout << x->mx << " " << x->mn << " " << x->processTime << " " << x->period <<endl;
		cout << ((float)x->mx - (float)x->mn)/(float)x->period << ',';
	}
	cout << endl;
// 	To find the statistics of the scheduler like the number of processes that missed the deadline  
	
	int numDeadlineMissed=0;
	float avgWaitTime=0;
	for (int i=0;i<c1.readyqueueCumEventqueue.size();i++){
		if (c1.readyqueueCumEventqueue[i]->missedDeadline>=1) numDeadlineMissed+=c1.readyqueueCumEventqueue[i]->missedDeadline;
		avgWaitTime+=c1.readyqueueCumEventqueue[i]->waitTime;
	}
	avgWaitTime/=c1.readyqueueCumEventqueue.size();
	statsFile1<<"Number of processes that entered the cpu "<<totalprocesses<<"\n";
	statsFile1<<"Number of processes that missed Deadline = "<<numDeadlineMissed<<"\n";
	statsFile1<<"The average waiting time = "<<avgWaitTime<<"\n";
	statsFile1<<"Number of processes that completed without missing deadline "<<totalprocesses-numDeadlineMissed<<"\n";

	ifile.close();
	statsFile1.close();
	// cout << "Number of times process preempted " << c1.num_preempted << "\n";
	return 0;
}
