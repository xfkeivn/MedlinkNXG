#pragma once
#define MAX_VIDEO_BUFFER_SIZE (4*1920*1080)
typedef enum 
{
	WAIT,
	EXIT,
	PUSH,
}Command;
typedef struct _SharedMem
{
	Command command;
	DWORD	tickcount;
	char frameBuffer[MAX_VIDEO_BUFFER_SIZE];

}SharedMem;