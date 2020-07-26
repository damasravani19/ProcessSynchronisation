#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

long long int no_of_chefs,no_of_servingtables,no_of_students,no_of_waitingstudents,no_of_studentsarrived,students,students_yettobe_arrived;
struct chef{
    int time;
    int vessels;
    int persons;
}Chef[1000];

struct st{
    int state;
    int currentcapacity; // 0->empty ; 1->filled
} Servingtable[1000];

struct stu{
    int state; // -1->not arrives 0->wait ; 1->allotes a slot 2-> served and left
    int servedby;
} Student[1000];

pthread_t servingtablepid[1000];
pthread_t studentpid[1000];
pthread_mutex_t lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;


void  biryani_ready(int arg)
{
  while(Chef[arg].vessels>0)
  {
    for(int i=1;i<=no_of_servingtables;i++)
    {
       //pthread_mutex_t lock; 
       pthread_mutex_lock(&lock);
       if(Servingtable[i].state==0 && Chef[arg].vessels>0)
       {
           //printf("bduhduq");
            Servingtable[i].state=1;
            Servingtable[i].currentcapacity=Chef[arg].persons;
            Chef[arg].vessels=Chef[arg].vessels-1;
            // break;
            printf("Serving Container of Table %d is refilled by Robot Chef %d; Table %d resuming serving now\n",i,arg,i);
           // biryani_ready_to_serve(&servingtablepid[i]);
       }
      pthread_mutex_unlock(&lock);
    }
   }
}

void * prepare_biryani(long long int arg)
{
    while(students<no_of_students)
    { 
    long long int w,r,p; // w-seconds r-vessels p-persons
    //for safety while updating the array mutex can be used but doesn't make much differnce as each chef have his own struct element 
    // all the threads are running simutaneouly(almost) depending on wait function there exit from the function depends
       Chef[arg].time=(rand()%(5-2+1))+2;
       Chef[arg].vessels=(rand()%(10-1+1))+1;
       Chef[arg].persons=(rand()%(50-25+1))+25;
        printf("Robot Chef %d is preparing %d vessels of Biryani\n",arg, Chef[arg].vessels);
        sleep(Chef[arg].time);
        printf("Robot Chef %d has prepared %d vessels of Biryani. Waiting for all the vessels to be emptied to resume cooking\n",arg, Chef[arg].vessels);
        //printf("%d w%d r%d p%d\n",arg,Chef[arg].time,Chef[arg].vessels,Chef[arg].persons);
        biryani_ready(arg);
        printf("All the vessels prepared by Robot Chef %d are emptied. Resuming cooking now.\n",arg);
    // return after all the vessels cooked bbby him are empty
    }
}

void student_in_slot(int arg){
    
    printf("Student %d on serving table %d has been served.\n",arg,Student[arg].servedby);
   //Student[arg].state=1;
    sleep(1);
  //  printf("Student %d has been served\n",arg);
    Student[arg].state=2;
    students++;
    return;
}
void * ready_to_serve(int arg)
{
    //each serving table has its own thread
    while(1)
    {
        int x;
        x=Servingtable[arg].state;
        //printf("x=%d\n",x);
        if(x==1)
        {
            printf("Serving Table %d is ready to serve with %d slots\n",arg,Servingtable[arg].currentcapacity);
          // generate sorts and then distributing them only when load is available else keep waiting for the load
          while(Servingtable[arg].currentcapacity>0 )
          {
              if(no_of_waitingstudents>0)
              {
               int slots;
               slots=(rand()%(10-1+1))+1;
               if(slots>Servingtable[arg].currentcapacity) { slots=Servingtable[arg].currentcapacity; }
               //printf("Container %d Slots %d\n",arg,slots);
               while(no_of_waitingstudents>0 && slots>0)
               {
                   for(int m=1;m<=no_of_studentsarrived && no_of_waitingstudents>0 && slots>0;m++)
                   {
                       pthread_mutex_lock(&lock1);
                       if(Student[m].state==0)
                       {
                           printf("Student %d assigned a slot on the serving table %d and waiting to be served\n",m,arg);
                           slots--;
                           Servingtable[arg].currentcapacity--;
                           no_of_waitingstudents--;
                           Student[m].servedby=arg;
                           student_in_slot(m);
                       }
                       pthread_mutex_unlock(&lock1);
        
                   }   
               }

              //printf("remaining sevice capacity %d  %d\n",Servingtable[arg].currentcapacity, no_of_waitingstudents);
               //no_of_students=no_of_students-slots;
               //Servingtable[arg].currentcapacity=Servingtable[arg].currentcapacity-slots;
              }
          }

          Servingtable[arg].state=0;
          printf("Serving Container of Table %d is empty, waiting for refill\n",arg);
        }
    }
}

void * wait_for_slot(int arg){
    pthread_mutex_lock(&lock2);
    Student[arg].state=0; 
    printf("Student %d is waiting to be allocated a slot on the serving table\n",arg);
    pthread_mutex_unlock(&lock2);
    
    int y;
    while(Student[arg].state==0) {
        //do nthg once this condition is violated return from the function
        if(no_of_waitingstudents==0)return;
    }
   return;
}
int main()
{
    
    printf("enter the number of chefs: ");
    scanf("%lld",&no_of_chefs);
    printf("enter the number of servingtables: ");
    scanf("%lld",&no_of_servingtables);
    printf("enter the number of students: ");
    scanf("%lld",&no_of_students);
    
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);

    pthread_t chefpid[no_of_chefs+1];
    pthread_t servingtablepid[no_of_servingtables+1];
    
    for(int i=1;i<=no_of_servingtables;i++)
     { 
        Servingtable[i].state=0;
        long long int error; 
        error = pthread_create(&(servingtablepid[i]), NULL,ready_to_serve,i);  //creating empty threads
        if (error != 0) 
            printf("\nThread can't be created : [%s]", strerror(error)); 
        
     } 

    for(int i=0;i<no_of_chefs;i++)
    {
        long long int error; 
        error = pthread_create(&(chefpid[i]), NULL,prepare_biryani,i+1); 
        if (error != 0) 
            printf("\nThread can't be created : [%s]", strerror(error)); 
    }

    students_yettobe_arrived=no_of_students;
    int i;
    i=0;
    while(students_yettobe_arrived>0)
     { 
         int x,y;
         x=(rand()%(10-1+1))+1;
         if(x>students_yettobe_arrived) { x=students_yettobe_arrived;}
         students_yettobe_arrived=students_yettobe_arrived-x;
         y=(rand()%(3-1+1))+1;
        no_of_studentsarrived=no_of_studentsarrived+x;
        no_of_waitingstudents=no_of_waitingstudents+x;
        while(x>0)
        {
            i++;
            printf("Student %d has arrived\n",i);
        Student[i].state=-1;
        long long int error; 
        error = pthread_create(&(studentpid[i]), NULL,wait_for_slot,i);  //creating empty threads
        if (error != 0) 
            printf("\nThread can't be created : [%s]", strerror(error));
            x--; 
        }
        sleep(15);
     } 

    for(int i=1;i<=no_of_students;i++)
    {
        pthread_join(studentpid[i], NULL); 
    }
    printf("Simulation Done\n");
}
