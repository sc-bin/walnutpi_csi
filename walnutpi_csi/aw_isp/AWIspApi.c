#include "AWIspApi.h"

#define LOG_TAG    "[AWIspApi]"

#ifdef __cplusplus
extern "C" {
#endif

#include "device/isp_dev.h"
#include "isp_dev/tools.h"

#include "isp_events/events.h"
#include "isp_tuning/isp_tuning_priv.h"
#include "isp_manage.h"

#include "iniparser/src/iniparser.h"
#include "include/V4l2Camera/sunxi_camera_v2.h"

#include "isp.h"

#ifdef __cplusplus
}
#endif

#define ALOG(level, ...) \
        ((void)printf(LOG_TAG ": " __VA_ARGS__))
#define ALOGV(...)   ALOG("V", __VA_ARGS__)
#define ALOGD(...)   ALOG("D", __VA_ARGS__)
#define ALOGI(...)   ALOG("I", __VA_ARGS__)
#define ALOGW(...)   ALOG("W", __VA_ARGS__)
#define ALOGE(...)   ALOG("E", __VA_ARGS__)
#define LOG_ALWAYS_FATAL(...)   do { ALOGE(__VA_ARGS__); exit(1); } while (0)

#define CHECK_ISP_ID(isp_id) if (isp_id < 0) { return -1; }

#define MAX_ISP_NUM 4

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int awIspApiInit()
{
    media_dev_init();
    return 0;
}

static int awSetFocusRegions(int isp_id, int x1, int y1, int x2, int y2)
{
    int res = -1;
    struct v4l2_win_setting win;

    CHECK_ISP_ID(isp_id);
    win.metering_mode = AUTO_FOCUS_METERING_SPOT;
    win.coor.x1 = x1;
    win.coor.y1 = y1;
    win.coor.x2 = x2;
    win.coor.y2 = y2;

    res = isp_set_attr_cfg(isp_id, ISP_CTRL_AF_METERING, &win);
    ALOGD("####awSetAfRegions res:%d x1:%d y1:%d x2:%d y2:%d\n",
        res,win.coor.x1,win.coor.y1,win.coor.x2,win.coor.y2);
    return res;
}

static int awSetSceneMode(int isp_id, int mode)
{
    int ret = -1;
#if ISP_AI_SCENE_CONF
    CHECK_ISP_ID(isp_id);
    ret = isp_set_scene(isp_id, (scene_mode_t)mode);
    if (ret < 0) {
        ALOGE("F:%s, L:%d, set scene mode err!\n",__FUNCTION__, __LINE__);
        return ret;
    }
#endif
    return 0;
}

static int awSetAeRegions(int mIspId, int x1, int y1, int x2, int y2)
{
    int res = -1;
    CHECK_ISP_ID(mIspId);
    struct isp_h3a_coor_win win;
    win.x1 = x1;
    win.y1 = y1;
    win.x2 = x2;
    win.y2 = y2;

    res = isp_set_attr_cfg(mIspId, ISP_CTRL_AE_ROI, &win);
    ALOGD("####awSetAeRegions res:%d x1:%d y1:%d x2:%d y2:%d\n",
        res,  win.x1,win.y1,win.x2,win.y2);
    return res;
}

static int awIspGetIspId(int video_id)
{
    int id = -1;

    id = isp_get_isp_id(video_id);

    if (id > MAX_ISP_NUM - 1) {
        id = -1;
        ALOGE("F:%s, L:%d, get isp id err!\n",__FUNCTION__, __LINE__);
    }
    ALOGD("F:%s, L:%d, video%d --> isp%d\n",__FUNCTION__, __LINE__, video_id, id);
    return id;
}

static int awIspStart(int isp_id)
{
    int ret = -1;

    CHECK_ISP_ID(isp_id);
    pthread_mutex_lock(&lock);
    ret = isp_init(isp_id);
    pthread_mutex_unlock(&lock);
    if (ret < 0) {
        ALOGE("F:%s, L:%d, isp init err! ret:%d\n",__FUNCTION__, __LINE__, ret);
        return ret;
    }
    ret = isp_run(isp_id);
    if (ret < 0) {
        ALOGE("F:%s, L:%d, isp run err! ret:%d\n",__FUNCTION__, __LINE__, ret);
        return ret;
    }
    return 0;
}

static int awIspSetIspLargeImage(int isp_id)
{
    int ret = -1;

    isp_set_stitch_mode(isp_id, STITCH_2IN1_LINNER);

    if (isp_id % 2 == 0)
        ret = isp_set_sync((0x1 << isp_id) | (0x1 << (isp_id + 1)));
    else
        ret = isp_set_sync((0x1 << (isp_id - 1)) | (0x1 << isp_id));

    if (ret < 0) {
        ALOGE("F:%s, L:%d, set large image err! ret:%d\n",
                    __FUNCTION__, __LINE__, ret);
        return ret;
    }
    return 0;
}

static int awIspStop(int isp_id)
{
    int ret = -1;

    ret = isp_stop(isp_id);
    ret = isp_pthread_join(isp_id);
    ret = isp_exit(isp_id);
    if (ret < 0) {
        ALOGE("F:%s, L:%d, ret:%d\n",__FUNCTION__, __LINE__, ret);
        return ret;
    }
    return 0;
}

static int awIspWaitToExit(int isp_id)
{
    int ret = -1;

    CHECK_ISP_ID(isp_id);
    ret = isp_pthread_join(isp_id);
    ret = isp_exit(isp_id);
    if (ret < 0) {
        ALOGE("F:%s, L:%d, ret:%d\n",__FUNCTION__, __LINE__, ret);
        return ret;
    }

    return 0;
}

static int awIspApiUnInit()
{
    media_dev_exit();
    return 0;
}

static int awSetFpsRanage(int isp_id, int fps)
{
    int ret = -1;

    CHECK_ISP_ID(isp_id);
    ret = isp_set_fps(isp_id, fps);
    return 0;
}

static int awIspGetIsoValue(int isp_id)
{
    int ret = -1;
    CHECK_ISP_ID(isp_id);
    ret = isp_get_iso_value(isp_id);
    return ret;
}

static int awIspGetExpTime(int isp_id, unsigned int* num, unsigned int* den)
{
    int ret = -1;
    CHECK_ISP_ID(isp_id);
    ret = isp_get_exp_time(isp_id, num, den);
    return ret;
}

static int awIspGetInfoLength(int isp_id, int *i3a_length, int *debug_length)
{
	return isp_get_info_length(i3a_length, debug_length);
}

static int awIspGet3AParameters(int isp_id, void *params)
{
	return isp_get_3a_parameters(isp_id, params);
}

static int awIspGetDebugMessage(int isp_id, void *msg)
{
	return isp_get_debug_msg(isp_id, msg);
}

AWIspApi *CreateAWIspApi(void)
{
	AWIspApi *ispport = (AWIspApi *)malloc(sizeof(AWIspApi));
	if (!ispport)
		return NULL;

    memset(ispport, 0, sizeof(AWIspApi));

    ispport->ispApiInit = awIspApiInit;
    ispport->ispSetAfRegions = awSetFocusRegions;
    ispport->ispSetSceneMode = awSetSceneMode;
    ispport->ispSetAeRegions = awSetAeRegions;
    ispport->ispGetIspId = awIspGetIspId;
    ispport->ispStart = awIspStart;
    ispport->ispSetIspLargeImage = awIspSetIspLargeImage;
    ispport->ispStop = awIspStop;
    ispport->ispWaitToExit = awIspWaitToExit;
    ispport->ispApiUnInit = awIspApiUnInit;
    ispport->ispSetFpsRanage = awSetFpsRanage;
    ispport->ispGetIspGain = awIspGetIsoValue;
    ispport->ispGetIspExp = awIspGetExpTime;
    ispport->ispGetInfoLength = awIspGetInfoLength;
    ispport->ispGet3AParameters = awIspGet3AParameters;
    ispport->ispGetDebugMessage = awIspGetDebugMessage;

	return ispport;
}

void DestroyAWIspApi(AWIspApi *hdl)
{
    if (hdl)
        free(hdl);
}