/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              uC/OS-II
*                                            EXAMPLE CODE
*
* Filename : main.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_mem.h>
#include  <os.h>

#include  "app_cfg.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#define TASK_STACKSIZE              2048
static  OS_STK  StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void task(void* p_arg);
static  void  StartupTask (void  *p_arg);


/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

int  main (void)
{
#if OS_TASK_NAME_EN > 0u
    CPU_INT08U  os_err;
#endif


    CPU_IntInit();

    Mem_Init();                                                 /* Initialize Memory Managment Module                   */
    CPU_IntDis();                                               /* Disable all Interrupts                               */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    OSInit();                                                   /* Initialize uC/OS-II                                  */
    
    OutFileInit();                                              /*Initialize Output File*/
    
    InputFile();                                                /*Input File*/
    /*Dynamic Create the Stack size*/
    Task_STK = malloc(TASK_NUMBER * sizeof(int*));

    /* for each pointer, allocate storage for an array of ints */
    int n;
    for (n = 0; n < TASK_NUMBER; n++) {
        Task_STK[n] = malloc(TASK_STACKSIZE * sizeof(int));
    }
 
    /* For RM scheduling : need to sort the period for priority of task.
       The task which has longest period has the lowest priority.
    */
    int i, j, key;
    task_para_set tmp_TaskParameter;
    for (i = 1; i < TASK_NUMBER; i++)       /* Start from the second task to sort, so i =1 */
    {
        key = TaskParameter[i].TaskPeriodic;
        tmp_TaskParameter = TaskParameter[i];
        j = i - 1;
        while (i >= 0 && TaskParameter[i].TaskPeriodic > key) /* Compare the period. If > key, move to the behind */
        {
            TaskParameter[i + 1] = TaskParameter[i];
            i = i - 1;
        }
        TaskParameter[i + 1] = tmp_TaskParameter; /* Insert the tmp */
    }


    // PA1: Create the task
    for (n = 0; n < TASK_NUMBER; n++)
    {
        OSTaskCreateExt(task,                            // Task Function   
            &TaskParameter[n],                            // p_arg
            &Task_STK[n][TASK_STACKSIZE - 1],             // ptop
            n + 1,                                        // prio (PA1 asks the priority start from 1)
            TaskParameter[n].TaskID,                      // id
            &Task_STK[n][0],                              // pbos
            TASK_STACKSIZE,                               // stack size
            &TaskParameter[n],                            // pext(TCB extension's pointer)
            (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR)); // opt
    }
    



/*   OSTaskCreateExt( StartupTask,                               /* Create the startup task*/
        /*            0,
        & StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE - 1u],
        APP_CFG_STARTUP_TASK_PRIO,
        APP_CFG_STARTUP_TASK_PRIO,
        & StartupTaskStk[0u],
        APP_CFG_STARTUP_TASK_STK_SIZE,
        0u,
        (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if OS_TASK_NAME_EN > 0u
        OSTaskNameSet(APP_CFG_STARTUP_TASK_PRIO,
            (INT8U*)"Startup Task",
            &os_err);
#endif
*/
    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    while (DEF_ON) {                                            /* Should Never Get Here.                               */
        ;
    }
}


/*
*********************************************************************************************************
*                                            STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'StartupTask()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

void task(void* p_arg) 
{
    task_para_set* task_data;
    task_data = p_arg;
    
    int TaskRdyTime = task_data->TaskArriveTime;
    int TaskRdyTimeNext, TaskExtTime, TaskStartTime, TaskDelayTime;

    if (OSTimeGet() < TaskRdyTime)
    {
        OSTimeDly(TaskRdyTime - OSTimeGet());
    }

    while (1)
    {
        OSTCBCur->SelfContinue = 0;
        TaskRdyTimeNext = TaskRdyTime + task_data->TaskPeriodic;
        OSTCBCur->TaskTimeNext = TaskRdyTimeNext;
        TaskExtTime = task_data->TaskExecutionTime;
        TaskStartTime = OSTimeGet();

        while (1) /*if the task return from Interrput*/
        {
            if (OSTimeGet() != TaskStartTime)
            {
                TaskExtTime--;
                OSTCBCur->RemainExeTime = TaskExtTime;
                if (TaskExtTime == 0)
                {
                    break;
                }
                TaskStartTime = OSTimeGet();
            }
        }

        TaskRdyTime = TaskRdyTimeNext; // Pre-update 

        if (OSTCBCur->RdyDelay == 1)
        {
            OSTCBCur->RdyDelay = 0;
            continue;
        }

        TaskDelayTime = TaskRdyTimeNext - OSTimeGet();
        if (TaskDelayTime == 0)
        {
            OSTCBCur->SelfContinue = 1;
            OS_Sched();
        }
        /*else if (TaskDelayTime < 0)
        {
            printf("TaskID %2d miss deadline at tick %2d !\n", task_data->TaskID, OSTimeGet());
        }*/

        OSTimeDly(TaskDelayTime);
    }
}

/*void task2(void* p_arg)
{
    task_para_set* task_data;
    task_data = p_arg;
    while (1)
    {
        
        printf("Tick: %d, Hello from task%d\n", OSTime, task_data->TaskID);
        if ((Output_err = fopen_s(&Output_fp, "./Output.txt", "a")) == 0)
        {
            fprintf(Output_fp, "Tick: %d, Hello from task%d\n", OSTime, task_data->TaskID);
            fclose(Output_fp);
        }
        OSTimeDly(task_data->TaskPeriodic);
    }
}*/

static  void  StartupTask (void *p_arg)
{
   (void)p_arg;

    OS_TRACE_INIT();                                            /* Initialize the uC/OS-II Trace recorder               */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
    
    APP_TRACE_DBG(("uCOS-III is Running...\n\r"));

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        OSTimeDlyHMSM(0u, 0u, 1u, 0u);
		APP_TRACE_DBG(("Time: %d\n\r", OSTimeGet()));
    }
}

