
#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/ata.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>

#include <drivers/amd_am79c973.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>


// #define GRAPHICSMODE


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;
using namespace myos::net;



void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void sleep(int seconds) 
{
    long int delay = 100000000; // 1 second delay
    // Loop until the desired amount of time has passed
    for (long int i = 0; i < seconds * delay; ++i)
        i++;
}

void printDigit(int digit) 
{
    char buff[256];
    int n; 
    int i;
    
    // check if the digit is positive or negative
    if (digit < 0) {
        digit *= -1;
        buff[0] = '-';
        i = n = 1;
    }
    else {
        i = n = 0;
    }

    do {
        buff[n] = '0' + (digit % 10);
        digit /= 10;
        ++n;
    } while (digit > 0);

    buff[n] = '\0';
    
    while (i < n / 2) {
        int temp = buff[i];
        buff[i] = buff[n - i - 1];
        buff[n - i - 1] = temp;
        ++i;        
    }
    printf((char *) buff);
}

void printArr(int * arr, int size)
{
    for (int i = 0; i < size; ++i) {
        printDigit(arr[i]); 
        printf(" ");
    }
    printf("\n");
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}
void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}
void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}





class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};

class PrintfUDPHandler : public UserDatagramProtocolHandler
{
public:
    void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size)
    {
        char* foo = " ";
        for(int i = 0; i < size; i++)
        {
            foo[0] = data[i];
            printf(foo);
        }
    }
};


class PrintfTCPHandler : public TransmissionControlProtocolHandler
{
public:
    bool HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, common::uint8_t* data, common::uint16_t size)
    {
        char* foo = " ";
        for(int i = 0; i < size; i++)
        {
            foo[0] = data[i];
            printf(foo);
        }
        
        
        
        if(size > 9
            && data[0] == 'G'
            && data[1] == 'E'
            && data[2] == 'T'
            && data[3] == ' '
            && data[4] == '/'
            && data[5] == ' '
            && data[6] == 'H'
            && data[7] == 'T'
            && data[8] == 'T'
            && data[9] == 'P'
        )
        {
            socket->Send((uint8_t*)"HTTP/1.1 200 OK\r\nServer: MyOS\r\nContent-Type: text/html\r\n\r\n<html><head><title>My Operating System</title></head><body><b>My Operating System</b> http://www.AlgorithMan.de</body></html>\r\n",184);
            socket->Disconnect();
        }
        
        
        return true;
    }
};

void collatzSeq(int n, int * buff) 
{
    int i = 0;
    buff[0] = n;
    while (n > 1) {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1; 
        buff[++i] = n;
    }
}

void printSeq(int * seq) 
{
    printDigit(*seq); printf(": ");
    while (*(++seq) > 1) {
        printDigit(*seq); printf(" ");
    }

    printDigit(1); printf("\n");
}

void entrypointCollatz() 
{
    int buff[256];

    for (int i = 25; i > 0;--i) {
        collatzSeq(i, buff);
        printSeq(buff);
    }
    sleep(1);
}

int linearSearch(int * arr, int size, int target) 
{
    for (int i = 0; i < size; ++i)
        if (arr[i] == target) 
        return i;
    return -1;
}

void entrypointLinearSearch() 
{
    int arr[] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
    int size = sizeof(arr) / sizeof(int);
    int target = 175;

    // SyscallHandler::sys_waitpid(4);

    printf("array : "); printArr(arr, size);
    printf("target: "); printDigit(target); printf("\n");
    int i = linearSearch(arr, size, target);
    printf("linear seach output: "); printDigit(i); printf("\n");
}

int rand(int max) 
{
    int r;
    return r % max; 
}

void swap(int* a, int* b) 
{
    int t = *a;
    *a = *b;
    *b = t;
}
 
int partition(int arr[], int low, int high) 
{
    int pivot = arr[high];
    int i = (low - 1);
  
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
 
void quickSort(int * arr, int low, int high) 
{
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int binarySearch(int * arr, int target, int lo, int hi) 
{
    if (lo > hi)
        return -1;

    int mid = (hi + lo) / 2;
    if (arr[mid] == target)
        return mid;
    else if (target < arr[mid])
        return binarySearch(arr, target, lo, mid - 1);
    else
        return binarySearch(arr, target, mid + 1, hi);
}

void entrypointBinarySearch() 
{
    int arr[] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
    int size = sizeof(arr) / sizeof(int);
    int target = 110;

    // first sort and print the array
    quickSort(arr, 0, size - 1);
    printf("sorted array: "); printArr(arr, size);
    printf("target: "); printDigit(target); printf("\n");
    int i = binarySearch(arr, target, 0, size - 1);
    printf("binary seach output: "); printDigit(i); printf("\n");
}

void entrypointInit() {
    while(true);
} 

// init process for 1st lifecycle
void init() 
{
    // fork three task
    int pid = 51; 
    SyscallHandler::sys_fork();
    printf("pid: "); printDigit(pid); printf("\n");
    sleep(5);

    if (pid != 0) {
        /* child code */
        SyscallHandler::sys_execve(entrypointBinarySearch);
    }
    else {
        /* parent code */
        // make another fork
        SyscallHandler::sys_fork();
        if (pid != 0) {
            /* child code */
            SyscallHandler::sys_execve(entrypointLinearSearch);
        }
        else {
            /* parent code */
            // make another fork
            SyscallHandler::sys_fork();
            if (pid != 0) {
                /* child code */
                SyscallHandler::sys_execve(entrypointCollatz);
            }
            else {
                while(true) /* infinite loop */ ;
            }
        }    
    }
}

void lifecyle1(GlobalDescriptorTable *gdt, TaskManager * taskManager)
{
    Task * taskInit = new Task(gdt, entrypointInit, 0, High);
    Task * taskLinearSearch = new Task(gdt, entrypointLinearSearch, 1, Normal);
    Task * taskBinarySearch = new Task(gdt, entrypointBinarySearch, 1, Normal);
    Task * taskCollatz = new Task(gdt, entrypointCollatz, 2, Normal);

    taskManager->AddTask(taskInit);
    taskManager->AddTask(taskLinearSearch);
    taskManager->AddTask(taskBinarySearch);
    taskManager->AddTask(taskCollatz);
}

void lifecyle2(GlobalDescriptorTable *gdt, TaskManager * taskManager)
{
    const int NUM_OF_EXEC = 10;
    const int NUM_OF_PROG = 3;
    void (*prog[NUM_OF_PROG]) (void) = {entrypointLinearSearch, entrypointBinarySearch, entrypointCollatz};

    void (*entrypoint)(void) = prog[rand(NUM_OF_PROG)];

    // load init process
    taskManager->AddTask(new Task (gdt, entrypointInit, 0, High));
    // load randomly selected program in specified number of times
    for (int i = 0; i < NUM_OF_EXEC; ++i) {
        Task * t = new Task(gdt, entrypoint, 1, Normal);
        taskManager->AddTask(t);
    }
}

void lifecyle3(GlobalDescriptorTable *gdt, TaskManager * taskManager)
{
    const int NUM_OF_EXEC = 3;
    const int NUM_OF_PROG = 3;
    void (*prog[NUM_OF_PROG]) (void) = {entrypointLinearSearch, entrypointBinarySearch, entrypointCollatz};

    int r1 = rand(NUM_OF_PROG), r2 = rand(NUM_OF_PROG);
    while (r1 != r2) 
        r2 = rand(NUM_OF_PROG);
    
    // load init process
    taskManager->AddTask(new Task (gdt, entrypointInit, 0, High));

    // load randomly selected 2 program in specified number of times
    for (int i = 0; i < NUM_OF_EXEC; ++i) {
        Task * t1 =  new Task(gdt, prog[r1], 1, Normal);
        Task * t2 =  new Task(gdt, prog[r2], 1, Normal);
        taskManager->AddTask(t1);
        taskManager->AddTask(t2);
    }
}

void lifecyleTest(GlobalDescriptorTable *gdt, TaskManager * taskManager)
{
    const int NUM_OF_EXEC = 3;
    const int NUM_OF_PROG = 3;
    void (*prog[NUM_OF_PROG]) (void) = {entrypointLinearSearch, entrypointBinarySearch, entrypointCollatz};

    // add init process
    taskManager->AddTask(new Task (gdt, entrypointInit, 0, High));

    taskManager->AddTask(new Task(gdt, entrypointLinearSearch, 1, Normal)); // PID: 2
    taskManager->AddTask(new Task(gdt, entrypointBinarySearch, 2, Normal)); // PID: 3
    taskManager->AddTask(new Task(gdt, entrypointCollatz, 2, Normal)); // PID: 4

}

void testWaitpid(GlobalDescriptorTable * gdt, TaskManager * taskManager) 
{
    // uncomment waitpid sentence from the entrypointLinearSearch function
    lifecyle1(gdt, taskManager);
}

void testFork() 
{
    for (int i = 0; i < 50; ++i) {
        printDigit(i); 
        printf(" ");
    }
    printf("\n");
    SyscallHandler::sys_fork();
    printf("Helloo from fork\n"); 
    while (true) /* infinitive loop */;
}

void testExecve() 
{
    for (int i = 0; i < 50; ++i) {
        printDigit(i); 
        printf(" ");
    }
    printf("\n");
    SyscallHandler::sys_execve(entrypointCollatz);
    printf("This will not be executed\n"); 
    while (true) /* infinitive loop */;
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Hello World! --- http://www.AlgorithMan.de\n");

    GlobalDescriptorTable gdt;
    
    
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
/*
    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);
*/
    
    void* allocated = memoryManager.malloc(1024);
/*
    printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8 ) & 0xFF);
    printfHex(((size_t)allocated      ) & 0xFF);
    printf("\n");
*/
    
    TaskManager taskManager(&gdt);

// ************************ TEST CASES ************************

/*
*/
    Task taskInit(&gdt, entrypointInit, 0, High);
    Task taskLinearSearch(&gdt, entrypointLinearSearch, 1, Normal);
    Task taskBinarySearch(&gdt, entrypointBinarySearch, 1, Normal);
    Task taskCollatz(&gdt, entrypointCollatz, 1, Normal);

    taskManager.AddTask(&taskInit);
    taskManager.AddTask(&taskLinearSearch);
    taskManager.AddTask(&taskBinarySearch);
    taskManager.AddTask(&taskCollatz);

/*
    Task taskExecve(&gdt, testExecve, 0, Normal);
    taskManager.AddTask(&taskExecve);
*/

/*
    Task taskFork(&gdt, testFork, 0, Normal);
    taskManager.AddTask(&taskFork);
*/

/*
    testWaitpid();
*/

/*
    lifecyle1(&gdt, &taskManager);
*/

/* 
    lifecyle2(&gdt, &taskManager);
*/

/*
    lifecyle3(&gdt, &taskManager);
*/

/*
    lifecyleTest(&gdt, &taskManager);
*/


    InterruptManager interrupts(0x20, &gdt, &taskManager);
    
    drivers::MouseEventHandler MouseEventHandler;
    drivers::MouseDriver MouseDriver(&interrupts, &MouseEventHandler);

    drivers::KeyboardEventHandler KeyboardEventHandler;
    drivers::KeyboardDriver KeyboardDriver(&interrupts, &KeyboardEventHandler);
    
    SyscallHandler syscalls(&taskManager, &interrupts, 0x80);
    ProcessExecutionHandler execs(&taskManager, &interrupts, 0x06);

/*
    printf("Initializing Hardware, Stage 1\n");
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);
        
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        #ifdef GRAPHICSMODE
            VideoGraphicsArray vga;
        #endif
        
    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();
        
    printf("Initializing Hardware, Stage 3\n");

    #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif


    printf("\nS-ATA primary master: ");
    AdvancedTechnologyAttachment ata0m(true, 0x1F0);
    ata0m.Identify();
    
    printf("\nS-ATA primary slave: ");
    AdvancedTechnologyAttachment ata0s(false, 0x1F0);
    ata0s.Identify();
    ata0s.Write28(0, (uint8_t*)"http://www.AlgorithMan.de", 25);
    ata0s.Flush();
    ata0s.Read28(0, 25);
    
    printf("\nS-ATA secondary master: ");
    AdvancedTechnologyAttachment ata1m(true, 0x170);
    ata1m.Identify();
    
    printf("\nS-ATA secondary slave: ");
    AdvancedTechnologyAttachment ata1s(false, 0x170);
    ata1s.Identify();
    // third: 0x1E8
    // fourth: 0x168
                       
    amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);

    
    // IP Address
    uint8_t ip1 = 10, ip2 = 0, ip3 = 2, ip4 = 15;
    uint32_t ip_be = ((uint32_t)ip4 << 24)
                | ((uint32_t)ip3 << 16)
                | ((uint32_t)ip2 << 8)
                | (uint32_t)ip1;
    eth0->SetIPAddress(ip_be);
    EtherFrameProvider etherframe(eth0);
    AddressResolutionProtocol arp(&etherframe);    

    
    // IP Address of the default gateway
    uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
    uint32_t gip_be = ((uint32_t)gip4 << 24)
                   | ((uint32_t)gip3 << 16)
                   | ((uint32_t)gip2 << 8)
                   | (uint32_t)gip1;
    
    uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;
    uint32_t subnet_be = ((uint32_t)subnet4 << 24)
                   | ((uint32_t)subnet3 << 16)
                   | ((uint32_t)subnet2 << 8)
                   | (uint32_t)subnet1;
                   
    InternetProtocolProvider ipv4(&etherframe, &arp, gip_be, subnet_be);
    InternetControlMessageProtocol icmp(&ipv4);
    UserDatagramProtocolProvider udp(&ipv4);
    TransmissionControlProtocolProvider tcp(&ipv4);
    

    

    printf("\n\n\n\n");
    
    arp.BroadcastMACAddress(gip_be);
    
    
    PrintfTCPHandler tcphandler;
    TransmissionControlProtocolSocket* tcpsocket = tcp.Listen(1234);
    tcp.Bind(tcpsocket, &tcphandler);
    //tcpsocket->Send((uint8_t*)"Hello TCP!", 10);

    
    //icmp.RequestEchoReply(gip_be);
    
    //PrintfUDPHandler udphandler;
    //UserDatagramProtocolSocket* udpsocket = udp.Connect(gip_be, 1234);
    //udp.Bind(udpsocket, &udphandler);
    //udpsocket->Send((uint8_t*)"Hello UDP!", 10);
    
    //UserDatagramProtocolSocket* udpsocket = udp.Listen(1234);
    //udp.Bind(udpsocket, &udphandler);
*/
    MouseDriver.Activate();
    KeyboardDriver.Activate();
    interrupts.Activate();

    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
