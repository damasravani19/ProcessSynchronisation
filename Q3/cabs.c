#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

long long int no_of_cabs,no_of_riders,no_of_paymentservers,count,k;
struct rider_data{
    int state;
    int waittime;
    int cabtype;
    int ridetime;
    int waitflag;
    int gotcab;
    int assignedcab;
    int readyforpayment;  // 0-> not ready 1-> ready for the payment 2->server is alloted
    int assignedserver;
}Rider[1000];

struct cab_data{
    int state;
    int presentridetime; // 0-> wait state    1-> onprimerride    2-> onpoolone   3->onpoolfull  
}Cab[1000];

struct payment_data{
    int state; 
    int presentrider; 
}Payment[1000];

pthread_t riderpid[1000];
pthread_t waitingpid[1000];
pthread_t riderinpid[1000];
pthread_t riderafterpid[1000];
pthread_t paymentpid[1000];
pthread_mutex_t lock;
pthread_mutex_t lock1;

void * rider_waittime(int arg){
   // printf("inn %d\n",arg);
    sleep(arg);
    Rider[arg].waitflag=1;
   // printf("out %d\n",arg);
}
void * rider_wait(int arg)
{
    
       Rider[arg].cabtype=(rand()%(2-1+1))+1; // 1-> premier 2-> pool
       Rider[arg].waittime=(rand()%(10-5+1))+5;
       Rider[arg].ridetime=(rand()%(3-1+1))+1;
       printf("rider%d  ct%d wt%d rt%d\n",arg,Rider[arg].cabtype,Rider[arg].waittime,Rider[arg].ridetime);
       Rider[arg].waitflag=0;
       Rider[arg].gotcab=0;
       
       pthread_create(&(waitingpid[arg]), NULL,rider_waittime,Rider[arg].waittime);

       while(Rider[arg].waitflag==0 && Rider[arg].gotcab==0)
       {
           //printf("%d %d \n",Rider[arg].gotcab,Rider[arg].waitflag);
           if(Rider[arg].cabtype==1)
           {
               //printf("type 1: %d :\n",arg);
               for(int i=1;i<=no_of_cabs && Rider[arg].waitflag==0 &&  Rider[arg].gotcab==0;i++)
               {
                   pthread_mutex_lock(&lock);
                  // printf("cabnumber%d cabstate%d \n",i,Cab[i].state);
                   if(Cab[i].state==0)
                   { 
                        Cab[i].state=1; 
                        Cab[i].presentridetime= Rider[arg].ridetime;
                        Rider[arg].assignedcab=i;
                        printf("Rider %d has been assigned %d cab\n",arg,i); 
                        Rider[arg].gotcab=1; 
                    }
                    pthread_mutex_unlock(&lock);
                   //printf("%d %d \n",Rider[arg].gotcab,Rider[arg].waitflag);
               }   
           }
           
           else
           {
               //printf("type 2 : %d :",arg);
               for(int i=1;i<=no_of_cabs && Rider[arg].waitflag==0 &&  Rider[arg].gotcab==0;i++)
               {
                   pthread_mutex_lock(&lock);
                  // printf("cabnumber%d cabstate%d aaa \n",i,Cab[i].state);
                   if(Cab[i].state==2) 
                   { 
                      Cab[i].state=3;
                      Rider[arg].assignedcab=i;
                      printf("Rider %d has been assigned %d cab\n",arg,i); 
                      Rider[arg].gotcab=1; 
                   }
                   //printf("%d %d \n",Rider[arg].gotcab,Rider[arg].waitflag);
                    pthread_mutex_unlock(&lock); 
               }

               if(Rider[arg].gotcab==0)
               {
                   for(int i=1;i<=no_of_cabs && Rider[arg].waitflag==0 && Rider[arg].gotcab==0;i++)
                   {
                        pthread_mutex_lock(&lock);
                        // printf("cabnumber%d cabstate%d bbb \n",i,Cab[i].state);
                         if(Cab[i].state==0) 
                         {
                           Cab[i].state=2; 
                           Rider[arg].assignedcab=i;
                           printf("Rider %d has been assigned %d cab\n",arg,i); 
                            Rider[arg].gotcab=1;
                         }
                         pthread_mutex_unlock(&lock);
                        // printf("%d %d \n",Rider[arg].gotcab,Rider[arg].waitflag);
                   }
               }
           }         
       }

      if(Rider[arg].gotcab==1) { printf("out of the loop\n"); }
      else if(Rider[arg].waitflag==1) { printf("Rider %d waitime has exceeded\n",arg); count--; }
    
}


void * rider_in_cab(int arg)
{
    while(count>0)
    {
        int x,y;
        x=Rider[arg].assignedcab;
        if(x!=0)
        {
            sleep(Rider[arg].ridetime);
            if(Rider[arg].cabtype==1)  //if the rider is in priermer cab
            {
                pthread_mutex_lock(&lock);
                 y=Rider[arg].assignedcab;
                 Cab[y].state=0;      //Cab is free;
                 Rider[arg].readyforpayment=1; 
                 if( Rider[arg].readyforpayment=1)
                 { 
                     printf("Rider %d ride has been completed\n",arg);
                     printf("Rider %d cab %d status after the ride is %d\n",arg,y,Cab[y].state);
                 }
                 pthread_mutex_unlock(&lock);
                 break;
            }  
            else 
            {
                pthread_mutex_lock(&lock);
                y=Rider[arg].assignedcab;
                if(Cab[y].state==2) { Cab[y].state=0;} // single pool
                else if(Cab[y].state==3) { Cab[y].state=2;}  // fullpool -> single pool
                Rider[arg].readyforpayment=1;
                pthread_mutex_unlock(&lock);
                if( Rider[arg].readyforpayment=1) 
                { 
                    printf("Rider %d ride has been completed\n",arg);
                    printf("Rider %d cab %d status after the ride is %d\n",arg,y,Cab[y].state);
                }
                break;
             }
        }
    }
}


/*void * payment_func(int arg)
{
    while(count>0)
    {
        int x;
        x=Payment[arg].state;
        if(x==1)
         {
            printf("%d server before sleep \n",arg); 
            sleep(2);
            count--;
            pthread_mutex_lock(&lock1);
            Payment[arg].state=0; 
            pthread_mutex_unlock(&lock1);
            printf("Payment of rider %d Has been completed from the server %d\n",Payment[arg].presentrider,arg);
            printf("%d server after sleep ; state after payment %d\n",arg,Payment[arg].state);
         }
    }
}*/
void * rider_after_cab(int arg)
{
    while(count>0)
    {

            if(Rider[arg].readyforpayment==1 && k>0)
            {
                int set;
                set=0;
                pthread_mutex_lock(&lock1);
                 if(k>=1) k--; else set=1;
                pthread_mutex_unlock(&lock1);
                if(set==0)
                {
                    sleep(2);
                    pthread_mutex_lock(&lock1);
                    k++;
                    pthread_mutex_unlock(&lock1);
                    count--;
                    Rider[arg].readyforpayment=2;
                    printf("Rider %d payment is completed\n",arg);
                }
            }
           // if(printf("Rider %d has been assigned the server %d",arg,i);) { printf("Rider %d payment is completed\n",arg);}
    }   
}
int main()
{
   printf("Enter the number of cabs : ");
   scanf("%lld",&no_of_cabs);
   printf("Enter the number of riders : ");
   scanf("%lld",&no_of_riders);
   count=no_of_riders;
   printf("Enter the number of payment servers : ");
   scanf("%lld",&no_of_paymentservers);
   k=no_of_paymentservers;

   pthread_mutex_init(&lock, NULL);
   pthread_mutex_init(&lock1, NULL);

   /*for(int i=1;i<=no_of_paymentservers;i++)
     { 
        Payment[i].state=0;
        long long int error; 
        error = pthread_create(&(paymentpid[i]), NULL,payment_func,i);  //creating empty threads
        if (error != 0) 
            printf("\nThread can't be created : [%s]", strerror(error)); 
        
     } */


   for(int i=1;i<=no_of_riders;i++)
   {
         int x;
         x=(rand()%(3-1+1))+1;
         sleep(x);
        printf("Rider %d came\n",i);
        long long int error; 
        error = pthread_create(&(riderpid[i]), NULL,rider_wait,i);  //creating empty threads
        if (error != 0) 
            printf("\nThread can't be created : [%s]", strerror(error)); 
        long long int error1; 
        Rider[i].assignedcab=0;
        error1 = pthread_create(&(riderinpid[i]), NULL,rider_in_cab,i);  //creating empty threads
        if (error1 != 0) 
            printf("\nThread can't be created : [%s]", strerror(error));
        long long int error2; 
        Rider[i].readyforpayment=0;
        error2= pthread_create(&(riderafterpid[i]), NULL,rider_after_cab,i);  //creating empty threads
        if (error2 != 0) 
            printf("\nThread can't be created : [%s]", strerror(error));
   }

   for(int i=1;i<=no_of_riders;i++)
    {
        pthread_join(riderpid[i], NULL); 
        
    }
    for(int i=1;i<=no_of_riders;i++)
    {
        pthread_join(riderinpid[i], NULL);
    }
    for(int i=1;i<=no_of_riders;i++)
    {
       pthread_join(riderafterpid[i], NULL);
    }
    for(int i=1;i<=no_of_paymentservers;i++)
    {
       pthread_join(paymentpid[i], NULL);
    }
    
   printf("Simulation Done\n");
}