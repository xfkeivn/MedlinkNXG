// FramePusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "assert.h"
#include <stdlib.h>
#include "CAgoraFramePusher.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <stdlib.h>
auto max_size = 1048576 * 20;
auto max_files = 20;
shared_ptr<spdlog::logger> logger;
int main(int argc,char* argv[])
{
	
	assert(argc == 9);


	char* app_id = argv[1];
	char* shm_name = argv[2];
	const char* width =argv[3];
	const char* height = argv[4];
	char* channel_id = argv[5];
	char* mutex_name = argv[6];
	char* video_source_name = argv[7];
	char* user_id = argv[8];
	string name = string(video_source_name) + ".log";
	logger = spdlog::rotating_logger_mt("medlinknxg", name.c_str(), max_size, max_files);
	logger->set_level(spdlog::level::debug);
	CAgoraFramePusher *pusher = new CAgoraFramePusher(app_id, shm_name, atoi(width), atoi(height), channel_id, mutex_name,atoi(user_id));
	logger->info("pusher started");
	
	pusher->StartShareMemoryCapture();
	logger->info("pusher off");
	delete pusher;

}


