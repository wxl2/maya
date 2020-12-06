//
// Created by wxl on 2020/11/30.
// 文件传输协议类型
//

#ifndef MAYA_FILEMSG_H
#define MAYA_FILEMSG_H

#include <stdint.h>

enum file_msg_type
{
    file_msg_type_unknown,
    msg_type_list_req,//获取文件列表
    msg_type_list_resp,
    msg_type_upload_req,//文件上传请求
    msg_type_upload_resp,//文件上传应答
    msg_type_download_req,
    msg_type_download_resp,
};

enum file_msg_error_code
{
    file_msg_error_unknown,//未知错误
    file_msg_error_progress,//文件正在上传或传输中
    file_msg_error_complete,//文件上传或下载完成
    file_msg_error_not_exist,//文件不存在
};


#pragma pack(push, 1)
//协议头
struct file_msg_header
{
    int64_t  packagesize;       //指定包体的大小
};

#pragma pack(pop)
/*!
 * 包构造
 * 命令头,表示当前的操作,文件包序列号(用于标识当前会话共发送了多少个数据包),错误码,文件名,当前传输数据包在文件中的偏移量,文件大小,文件内容
 * 客户端发生的数据不会有错误码
 * 当命令头为msg_type_list_*时,文件名为get,len=3,便宜量都应置为空
 */
#endif //MAYA_FILEMSG_H
