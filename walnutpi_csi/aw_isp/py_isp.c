#include <Python.h>
#include <structmember.h>
#include <pthread.h>
#include "AWIspApi.h"
#include <fcntl.h>
#include <unistd.h>

typedef struct
{
    int isp_id;
} thread_args_t;

void *isp_thread(void *arg)
{
    int original_stdout = dup(STDOUT_FILENO);
    int original_stderr = dup(STDERR_FILENO);
    
    // 重定向标准输出和错误输出到 /dev/null
    int dev_null = open("/dev/null", O_WRONLY);
    // dup2(dev_null, STDOUT_FILENO);
    // dup2(dev_null, STDERR_FILENO);
    // close(dev_null);
    
    thread_args_t *args = (thread_args_t *)arg;
    int isp_id = args->isp_id;

    int ret = 0;
    AWIspApi *isp;
    isp = CreateAWIspApi();

    isp->ispApiInit();
    isp_id = isp->ispGetIspId(0);
    printf("isp_id:%d\n", isp_id); 

    ret = isp->ispStart(isp_id);
    if (ret < 0)
    {
        printf("isp start err!\n"); 
    }

    isp->ispWaitToExit(isp_id);
    isp->ispApiUnInit();
    DestroyAWIspApi(isp);

    // 恢复原始的标准输出和错误输出
    // dup2(original_stdout, STDOUT_FILENO);
    // dup2(original_stderr, STDERR_FILENO);
    // close(original_stdout);
    // close(original_stderr);
    
    free(args);
    return NULL;
}

static PyObject *aw_isp_start(PyObject *self, PyObject *args)
{
    int isp_id;

    if (!PyArg_ParseTuple(args, "i", &isp_id))
    {
        return NULL;
    }

    thread_args_t *thread_args = malloc(sizeof(thread_args_t));
    if (!thread_args)
    {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory for thread arguments");
        return NULL;
    }
    thread_args->isp_id = isp_id;

    // 创建线程
    pthread_t thread;
    int result = pthread_create(&thread, NULL, isp_thread, thread_args);
    if (result != 0)
    {
        free(thread_args);
        PyErr_SetString(PyExc_RuntimeError, "Failed to create thread");
        return NULL;
    }

    // 分离线程，让系统自动回收资源
    pthread_detach(thread);

    Py_RETURN_NONE;
}

static PyMethodDef AwIspMethods[] = {

    {"start", aw_isp_start, METH_VARARGS,
     "Start ISP in a new thread with a number parameter"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef aw_isp_module = {
    PyModuleDef_HEAD_INIT,
    "_aw_isp",                        // 模块名
    "Allwinner ISP Python Extension", // 模块文档字符串
    -1,                               // 模块保持状态的大小
    AwIspMethods                      // 模块方法
};

PyMODINIT_FUNC PyInit__aw_isp(void)
{
    return PyModule_Create(&aw_isp_module);
}