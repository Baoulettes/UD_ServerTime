#include <unidokkan/hook.h>
#include <unidokkan/errors.h>
#include <unidokkan/ui/button.h>
#include <dokkan/instances.h>
#include <unidokkan/ui/imageview.h>
#include <dokkan/defs.h>
#include <unidokkan/ui/scene.h>
#include <unidokkan/ui/layout.h>
#include <dokkan/files.h>
#include <unidokkan/database.h>
#include <cocos/2d/CCActionInterval.h>
#include <dokkan/scheduler.h>
#include <atomic>

using namespace UniDokkan::UI;
namespace cocos2d = ud_cocos2d;
std::atomic<bool> kStopProgressThread{false};
std::string currentDateTime_pst() {
	setenv("TZ", "PST8PST", 1); 
	tzset();
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%r", &tstruct);
	// UD_LOGI("Time_Checker_layout : %s",buf);
	return buf;
}
std::string currentDateTime_local() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%r PST", &tstruct);
	// UD_LOGI("Time_Checker_layout : %s",buf);
	return buf;
}
bool *(*ori_Time_Checker_layout)(void *) = nullptr;
bool *Time_Checker_layout(void *scene) {
	std::string Current_hour=	"XX:XX PST";
	auto file_utils 		= 	UniDokkan::Dokkan::cocos2d_FileUtils_getInstance();
    auto screen_size		= 	cocos2d_Director_getVisibleSize(UniDokkan::Dokkan::cocos2d_Director_getInstance());
	auto OriW				=	screen_size.width;
	auto OriH				=	screen_size.height;
	float Ypos				=	OriH/2;
	float XPos				=	OriW/2;
	float YposN				=	Ypos + 465.0f;
	float XPosN				=	XPos - 190.0f;
	float font_size			=	20.0f;
	std::string font_path	=	"baou_st/FOT-NewRodinProN-B.otf";
	Layout Background_Server_time("Background_Server_time", scene, cocos2d::ui::Layout::Type::RELATIVE,100000);
	Background_Server_time.setContentSize(screen_size);
	Background_Server_time.setPosition({0, 0});

	Layout Background_Server_time_Layout("Background_Server_time_Layout", Background_Server_time, cocos2d::ui::Layout::Type::ABSOLUTE,1);
	Background_Server_time_Layout.setPosition({XPos, Ypos});
	Background_Server_time_Layout.setContentSize(screen_size);
	
	Label Server_time_label("Server_time", Background_Server_time_Layout,1000);
	Server_time_label.setTTFFont(font_path, font_size);
	Server_time_label.setPosition({XPosN,YposN});
	Server_time_label.enableItalics(true);
	Server_time_label.setAdditionalKerning(0.0f);
	Server_time_label.enableOutline(true, cocos2d::Color4B::BLACK, 1.0f);
	Server_time_label.setCaption(Current_hour);
	Server_time_label.setAlignment(cocos2d::TextHAlignment::CENTER, cocos2d::TextVAlignment::BOTTOM);

	std::thread Server_Time([=]() mutable {
		// Scheduler provides the performFunctionInCocosThread()
		UniDokkan::Dokkan::Scheduler scheduler;

		// Loop while the stop flag is false
		while (!kStopProgressThread) {
			// Sleep this thread for 1 second
			std::this_thread::sleep_for(std::chrono::seconds(1));

			// setCaption the Server_time_label in the cocos2d main thread
			scheduler.performFunctionInCocosThread([=]() mutable {
				Current_hour	= 	currentDateTime_pst();
				Server_time_label.setCaption(Current_hour);
			});
		}
	});
	Server_Time.detach();
	
	UD_LOGI("UI Server_time_label");
	return ori_Time_Checker_layout(scene);
}

extern "C" {
    [[maybe_unused]] __attribute__ ((visibility ("default")))
    int unidokkan_init_v4(HookLibV4* hook_lib) {
        UD_LOGI("Server_Time module loading...");

        if (hook_lib->size < sizeof(HookLibV4)) {
            return UD_MODULE_INVALID_SIZE;
        }

        if (hook_lib->version < kMinPatcherVer) {
            return UD_MODULE_INVALID_VERSION;
        }

        auto res = hook_lib->applyHooks(
            {
				{DOKKAN_LIB, "_ZN11HeaderLayer4initEv", Time_Checker_layout, &ori_Time_Checker_layout},
            }
        );

        if (!res) {
            return UD_MODULE_ERROR;
        }

        UD_LOGI("Server_Time module successfully loaded.");
        return UD_MODULE_SUCCESS;
    }

}
