
#include <syscalls.h>

using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;
 
 


SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
:    InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}

void fork();
void waitpid(uint32_t*);
void execve();
void printf(char*);

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    

    switch(cpu->eax)
    {
		case 2: 
			fork();
			break;
        case 4:
            printf((char*)cpu->ebx);
            break;
        case 7:
            waitpid(&esp); // unsigned int*
            break;
        case 11:
            execve();
            break;
        default:
            break;
    }

    
    return esp;
}
