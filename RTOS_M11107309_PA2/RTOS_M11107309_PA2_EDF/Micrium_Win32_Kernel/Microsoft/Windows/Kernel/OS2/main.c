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

#define TASK_STACKSIZE 2048
static  OS_STK  StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static void task1(void* p_arg);
static void task2(void* p_arg);

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

    /// initialize Output file
    OutFileInit();

    /// load the input file
    InputFile();
    /// initialize Task Stack
    Task_STK = malloc(TASK_NUMBER * sizeof(int*));
    int n;
    for (n = 0;n < TASK_NUMBER;n++) 
    {
        Task_STK[n] = malloc(TASK_STACKSIZE * sizeof(int));
    }

    /// #PA1
    /// RM scheduling rules : task with smallest time period will have highest priority 
    /// insertion sort by TaskPeriodic => TaskParameter[0]??????period?????p??????Task???A?????????C
    //int i, j, key;
    //task_para_set tmp_TaskParameter;
    //for (j = 1;j < TASK_NUMBER;j++)
    //{
    //    key = TaskParameter[j].TaskPeriodic;
    //    tmp_TaskParameter = TaskParameter[j];
    //    i = j - 1;
    //    while (i >= 0 && TaskParameter[i].TaskPeriodic > key)
    //    {
    //        TaskParameter[i + 1] = TaskParameter[i];
    //        i = i - 1;
    //    }
    //    TaskParameter[i + 1] = tmp_TaskParameter;
    //}
    /// #PA1

    /// PA#2 Add 3 argument
    for (n = 0;n < TASK_NUMBER;n++) 
    {
        /// printf("ID %d, prio %d\n", TaskParameter[n].TaskID, n+1);
        OSTaskCreateExt(task1,                                                  /// task function
            &TaskParameter[n],                                                  /// p_arg
            &Task_STK[n][TASK_STACKSIZE - 1],                                   /// ptos
            n+1,                                                                /// prio, PA#1 start from 1 
            TaskParameter[n].TaskID,                                            /// id
            &Task_STK[n][0],                                                    /// pbos
            TASK_STACKSIZE,                                                     /// stack size
            &TaskParameter[n],                                                  /// pext(TCB extentsion's pointer)
            (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR),                        /// opt
            TaskParameter[n].TaskArriveTime,                                        /// arrive_time
            TaskParameter[n].TaskExecutionTime,                                     /// exe_time
            TaskParameter[n].TaskPeriodic                                           /// period
        );   
    }

    /// PA#1 part1
    //printf("============ TCB linked list ============\n");
    //printf("Task\tPrev_TCB_addr\tTCB_addr\tNext_TCB_addr\n");
    //for (OS_TCB* ptcb = OSTCBList; ptcb != (OS_TCB*)0; ptcb = ptcb->OSTCBNext)
    //{
    //    printf("%d\t%p\t%p\t%p\n", ptcb->OSTCBPrio, ptcb->OSTCBPrev, ptcb, ptcb->OSTCBNext);
    //}
    /// PA#1 part1


//    OSTaskCreateExt( StartupTask,                               /* Create the startup task                              */
//                     0,
//                    &StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE - 1u],
//                     APP_CFG_STARTUP_TASK_PRIO,
//                     APP_CFG_STARTUP_TASK_PRIO,
//                    &StartupTaskStk[0u],
//                     APP_CFG_STARTUP_TASK_STK_SIZE,
//                     0u,
//                    (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

//#if OS_TASK_NAME_EN > 0u
//    OSTaskNameSet(         APP_CFG_STARTUP_TASK_PRIO,
//                  (INT8U *)"Startup Task",
//                           &os_err);
//#endif

    OSTimeSet(0);
    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    while (DEF_ON) {                                            /* Should Never Get Here.                               */
        ;
    }
}

void task1(void* p_arg) {
    task_para_set* task_data;
    task_data = p_arg;

    while (1)
    {
    }
}

void task2(void* p_arg) {
    task_para_set* task_data;
    task_data = p_arg;
    while (1)
    {
        //printf("Tick: %d, Hello from task%d\n", OSTime, task_data->TaskID);
        //if ((Output_err = fopen_s(&Output_fp, "./Output.txt", "a")) == 0)
        //{
        //    //fprintf(Output_fp, "Tick: %d, Hello from task%d\n", OSTime, task_data->TaskID);
        //    //fclose(Output_fp);
        //}
        OSTimeDly(task_data->TaskPeriodic);
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

